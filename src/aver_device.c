//
// Created by covers1624 on 5/09/24.
//

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <linux/uvcvideo.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "aver_device.h"

#define RAW_LED_WRITE 0x01
#define RAW_HDCP_WRITE 0x10

#define RAW_LED_OFF 0x13
#define RAW_LED_25 0x14
#define RAW_LED_50 0x15
#define RAW_LED_75 0x16
#define RAW_LED_100 0x17

#define RAW_HDCP_OFF 0x00
#define RAW_HDCP_ON 0x01

struct aver_control {
    uint8_t write_state; // returns 0xFF for get. Set to 0x10 for HDCP write, 0x01 for LED write.
    uint8_t led; // The LED state, 0x13-0x17.
    uint8_t unk3; // Always 0xFF for LED write or GET.
    uint8_t unk4; // Always 0xFF for LED write or GET.
    uint8_t unk5; // Always 0xFF for LED write or GET.
    uint8_t unk6; // Always 0xFF for LED write or GET.
    uint8_t unk7; // Always 0xFF for LED write or GET.
    uint8_t unk8; // Always 0xFF for LED write or GET.
    uint8_t unk9; // Always 0xFF for LED write or GET.
    uint8_t hdcp; // The HDCP state, 0x00-0x01
    uint8_t unk11; // Always 0xFF for LED write or GET.
    uint8_t unk12; // Always 0xFF for LED write or GET.
    uint8_t unk13; // Always 0x31 for LED write or GET.
};

bool aver_ioctl(int fd, int query, struct aver_control *data) {
    struct uvc_xu_control_query ioctl_query = {
            .unit = 0x04,
            .selector = 0x0b,
            .query = query,
            .size = sizeof(struct aver_control),
            .data = (uint8_t *) data
    };
    int ret = ioctl(fd, UVCIOC_CTRL_QUERY, &ioctl_query);
    if (ret < 0) {
        fprintf(stderr, "Failed to ioctl query. %s\n", strerror(errno));
        return false;
    }
    return true;
}

bool aver_set_hdcp(int fd, aver_hdcp_state state) {
    if (state == HDCP_UNKNOWN) return true; // Sure, why not.

    struct aver_control data;
    memset(&data, 0, sizeof(data));

    data.write_state = RAW_HDCP_WRITE;
    data.hdcp = state == HDCP_ON ? RAW_HDCP_ON : RAW_HDCP_OFF;
    if (!aver_ioctl(fd, 0x01, &data)) return false;

    return true;
}

bool aver_get_state(int fd, aver_state *state) {
    struct aver_control data;
    memset(&data, 0, sizeof(data));

    if (!aver_ioctl(fd, 0x81, &data)) return false;

    if (data.led >= RAW_LED_OFF && data.led <= RAW_LED_100) {
        state->led = data.led - RAW_LED_OFF;
    }
    if (data.hdcp == RAW_HDCP_OFF || data.hdcp == RAW_HDCP_ON) {
        state->hdcp = data.hdcp;
    }
    return true;
}

bool aver_set_led(int fd, aver_led_state state) {
    if (state == LED_UNKNOWN) return true; // Sure, why not.

    // Setting LED's appears to require the data from the query.
    struct aver_control data;
    memset(&data, 0, sizeof(data));
    if (!aver_ioctl(fd, 0x81, &data)) return false;

    // Only these things are changed in the LED packet.
    data.write_state = RAW_LED_WRITE;
    data.led = state + RAW_LED_OFF;
    if (!aver_ioctl(fd, 0x01, &data)) return false;

    return true;
}

bool aver_set_state(int fd, aver_state *state) {
    if (!aver_set_hdcp(fd, state->hdcp)) return false;
    if (!aver_set_led(fd, state->led)) return false;

    return true;
}

const char *aver_led_state_str(aver_led_state state) {
    switch (state) {
        case LED_OFF:
            return "OFF";
        case LED_25:
            return "25%";
        case LED_50:
            return "50%";
        case LED_75:
            return "75%";
        case LED_100:
            return "100%";
        default:
            return "UNKNOWN";
    }
}

const char *aver_hdcp_state_str(aver_hdcp_state state) {
    switch (state) {
        case HDCP_OFF:
            return "OFF";
        case HDCP_ON:
            return "ON";
        default:
            return "UNKNOWN";
    }
}
