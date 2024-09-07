//
// Created by covers1624 on 5/09/24.
//

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

#include "linux/videodev2.h"

#include <stdio.h>
#include <argp.h>

#include <string.h>
#include <strings.h>

#include "aver_device.h"

#ifndef VERSION
#define VERSION "vUNKNOWN"
#endif

const char *argp_program_version = "avm-cli " VERSION;
const char *argp_program_bug_address = "https://github.com/covers1624/avm-cli";
static char doc[] = "CLI application for controlling AVerMedia UVC devices.";
static char args_doc[] = "";
static struct argp_option options[] = {
        {"status", 's', NULL,     0, "Print the current device status.",                   0},
        {"device", 'd', "DEVICE", 0, "The video device to control.",                       1},
        {"hdcp",   'h', "HDCP",   0, "Set the current hdcp status. [1/0, on/off, yes/no]", 1},
        {"led",    'l', "LED",    0, "Set the current led status. [0, 25, 50, 75, 100]",   1},
        {NULL}
};

struct config {
    bool status;

    char *device;
    aver_hdcp_state set_hdcp;
    aver_led_state set_led;
};

static bool parse_hdcp_state(char *arg, aver_hdcp_state *state) {
    if (strcmp(arg, "1") == 0
        || strcasecmp(arg, "on") == 0
        || strcasecmp(arg, "yes") == 0) {
        *state = HDCP_ON;
        return true;
    }
    if (strcmp(arg, "0") == 0
        || strcasecmp(arg, "off") == 0
        || strcasecmp(arg, "no") == 0) {
        *state = HDCP_OFF;
        return true;
    }
    return false;
}

static bool parse_led_state(char *arg, aver_led_state *state) {
    char *endPtr;
    long val = strtol(arg, &endPtr, 10);
    if (errno != 0 || *endPtr != 0) { // Parser fail, or we did not parse the whole string.
        return false;
    }
    switch (val) {
        case 0:
            *state = LED_OFF;
            return true;
        case 25:
            *state = LED_25;
            return true;
        case 50:
            *state = LED_50;
            return true;
        case 75:
            *state = LED_75;
            return true;
        case 100:
            *state = LED_100;
            return true;
        default:
    }
    return false;
}

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct config *config = state->input;
    switch (key) {
        case 's':
            config->status = true;
            break;
        case 'd':
            config->device = strdup(arg);
            break;
        case 'h':
            if (!parse_hdcp_state(arg, &config->set_hdcp)) {
                argp_error(state, "Invalid HDCP value %s expected one of [1, on, yes] or [0, off, no].", arg);
            }
            break;
        case 'l':
            if (!parse_led_state(arg, &config->set_led)) {
                argp_error(state, "Invalid LED value %s expected one of [0, 25, 50, 75, 100].", arg);
            }
            break;
        case ARGP_KEY_END:
            if (config->device == NULL) {
                argp_error(state, "Device must be specified via -d\n");
            }
            if (!config->status && config->set_hdcp == HDCP_UNKNOWN && config->set_led == LED_UNKNOWN) {
                argp_error(state, "One of -s, -h, or -l is required.");
            }
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, NULL, NULL, NULL};

int main(int argc, char **argv) {
    struct config config = {
            .status = false,
            .set_hdcp = HDCP_UNKNOWN,
            .set_led = LED_UNKNOWN,
    };
    argp_parse(&argp, argc, argv, 0, NULL, &config);

    struct stat st;
    if (stat(config.device, &st) < 0) {
        fprintf(stderr, "Failed to stat file '%s'. %s \n", config.device, strerror(errno));
        return 1;
    }

    if (!S_ISCHR(st.st_mode)) {
        fprintf(stderr, "Provided file '%s' is not a device.\n", config.device);
        return 1;
    }

    int fd = open(config.device, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open device. %s\n", strerror(errno));
        return 0;
    }

    struct v4l2_capability caps;
    if (ioctl(fd, VIDIOC_QUERYCAP, &caps) < 0) {
        int err = errno;
        if (err == ENOTTY) {
            fprintf(stderr, "Device is not a V4L2 device.\n");
        } else {
            fprintf(stderr, "Failed to query V4L2 device capabilities. %d\n", errno);
        }
        close(fd);
        return 1;
    }
    if ((caps.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
        fprintf(stderr, "V4L2 device is not a capture device.\n");
        close(fd);
        return 1;
    }
    if (config.status) {
        fprintf(stderr, "Found card: %s (%s)\n", caps.card, caps.bus_info);
    }

    if (strstr((const char *) &caps.card, "AVerMedia USB") == NULL) {
        fprintf(stderr, "Device is not an AVerMedia capture device.\n");
        close(fd);
        return 1;
    }

    aver_state state;
    if (!aver_get_state(fd, &state)) {
        fprintf(stderr, "Failed to query device.\n");
    } else {
        if (config.status) {
            fprintf(stderr, "LED: %s\n", aver_led_state_str(state.led));
            fprintf(stderr, "HDCP: %s\n", aver_hdcp_state_str(state.hdcp));
        }
        state.hdcp = config.set_hdcp;
        state.led = config.set_led;
        if (!aver_set_state(fd, &state)) {
            fprintf(stderr, "Failed to set state.\n");
        }
    }

    close(fd);
    free(config.device);
    return 0;
}
