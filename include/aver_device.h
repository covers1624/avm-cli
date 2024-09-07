//
// Created by covers1624 on 5/09/24.
//

#pragma once

enum aver_led_state_t {
    LED_OFF,
    LED_25,
    LED_50,
    LED_75,
    LED_100,
    LED_UNKNOWN,
};
typedef enum aver_led_state_t aver_led_state;

const char *aver_led_state_str(aver_led_state state);

enum aver_hdcp_state_t {
    HDCP_OFF,
    HDCP_ON,
    HDCP_UNKNOWN,
};
typedef enum aver_hdcp_state_t aver_hdcp_state;

const char *aver_hdcp_state_str(aver_hdcp_state state);

struct aver_state_t {
    aver_led_state led;
    aver_hdcp_state hdcp;
};

typedef struct aver_state_t aver_state;

bool aver_get_state(int fd, aver_state *state);

bool aver_set_state(int fd, aver_state *state);
