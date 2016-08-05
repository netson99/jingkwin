/********************************************************************
KWin - the KDE window manager
This file is part of the KDE project.

Copyright (C) 2016 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include <libinput.h>
#include "mock_libinput.h"
#include <config-kwin.h>

#include <linux/input.h>

int libinput_device_keyboard_has_key(struct libinput_device *device, uint32_t code)
{
    return device->keys.contains(code);
}

int libinput_device_has_capability(struct libinput_device *device, enum libinput_device_capability capability)
{
    switch (capability) {
    case LIBINPUT_DEVICE_CAP_KEYBOARD:
        return device->keyboard;
    case LIBINPUT_DEVICE_CAP_POINTER:
        return device->pointer;
    case LIBINPUT_DEVICE_CAP_TOUCH:
        return device->touch;
    case LIBINPUT_DEVICE_CAP_GESTURE:
        return device->gestureSupported;
#if HAVE_LIBINPUT_1_2
    case LIBINPUT_DEVICE_CAP_TABLET_TOOL:
        return device->tabletTool;
#endif
    default:
        return 0;
    }
}

const char *libinput_device_get_name(struct libinput_device *device)
{
    return device->name.constData();
}

const char *libinput_device_get_sysname(struct libinput_device *device)
{
    return device->sysName.constData();
}

const char *libinput_device_get_output_name(struct libinput_device *device)
{
    return device->outputName.constData();
}

unsigned int libinput_device_get_id_product(struct libinput_device *device)
{
    return device->product;
}

unsigned int libinput_device_get_id_vendor(struct libinput_device *device)
{
    return device->vendor;
}

int libinput_device_config_tap_get_finger_count(struct libinput_device *device)
{
    return device->tapFingerCount;
}

enum libinput_config_tap_state libinput_device_config_tap_get_enabled(struct libinput_device *device)
{
    Q_UNUSED(device)
    return LIBINPUT_CONFIG_TAP_DISABLED;
}

enum libinput_config_tap_state libinput_device_config_tap_get_default_enabled(struct libinput_device *device)
{
    if (device->tapEnabledByDefault) {
        return LIBINPUT_CONFIG_TAP_ENABLED;
    } else {
        return LIBINPUT_CONFIG_TAP_DISABLED;
    }
}

int libinput_device_config_dwt_is_available(struct libinput_device *device)
{
    return device->supportsDisableWhileTyping;
}

int libinput_device_config_accel_is_available(struct libinput_device *device)
{
    return device->supportsPointerAcceleration;
}

int libinput_device_config_calibration_has_matrix(struct libinput_device *device)
{
    return device->supportsCalibrationMatrix;
}

int libinput_device_config_left_handed_is_available(struct libinput_device *device)
{
    return device->supportsLeftHanded;
}

uint32_t libinput_device_config_send_events_get_modes(struct libinput_device *device)
{
    uint32_t modes = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;
    if (device->supportsDisableEvents) {
        modes |= LIBINPUT_CONFIG_SEND_EVENTS_DISABLED;
    }
    if (device->supportsDisableEventsOnExternalMouse) {
        modes |= LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE;
    }
    return modes;
}

int libinput_device_config_left_handed_get(struct libinput_device *device)
{
    return device->leftHanded;
}

double libinput_device_config_accel_get_speed(struct libinput_device *device)
{
    return device->pointerAcceleration;
}

uint32_t libinput_device_config_send_events_get_mode(struct libinput_device *device)
{
    if (device->enabled) {
        return LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;
    } else {
        // TODO: disabled on eternal mouse
        return LIBINPUT_CONFIG_SEND_EVENTS_DISABLED;
    }
}

struct libinput_device *libinput_device_ref(struct libinput_device *device)
{
    return device;
}

struct libinput_device *libinput_device_unref(struct libinput_device *device)
{
    return device;
}

int libinput_device_get_size(struct libinput_device *device, double *width, double *height)
{
    if (device->deviceSizeReturnValue) {
        return device->deviceSizeReturnValue;
    }
    if (width) {
        *width = device->deviceSize.width();
    }
    if (height) {
        *height = device->deviceSize.height();
    }
    return device->deviceSizeReturnValue;
}

int libinput_device_pointer_has_button(struct libinput_device *device, uint32_t code)
{
    switch (code) {
    case BTN_LEFT:
        return device->supportedButtons.testFlag(Qt::LeftButton);
    case BTN_MIDDLE:
        return device->supportedButtons.testFlag(Qt::MiddleButton);
    case BTN_RIGHT:
        return device->supportedButtons.testFlag(Qt::RightButton);
    case BTN_SIDE:
        return device->supportedButtons.testFlag(Qt::ExtraButton1);
    case BTN_EXTRA:
        return device->supportedButtons.testFlag(Qt::ExtraButton2);
    case BTN_BACK:
        return device->supportedButtons.testFlag(Qt::BackButton);
    case BTN_FORWARD:
        return device->supportedButtons.testFlag(Qt::ForwardButton);
    case BTN_TASK:
        return device->supportedButtons.testFlag(Qt::TaskButton);
    default:
        return 0;
    }
}

enum libinput_config_status libinput_device_config_left_handed_set(struct libinput_device *device, int left_handed)
{
    if (device->setLeftHandedReturnValue == 0) {
        device->leftHanded = left_handed;
        return LIBINPUT_CONFIG_STATUS_SUCCESS;
    }
    return LIBINPUT_CONFIG_STATUS_INVALID;
}

enum libinput_config_status libinput_device_config_accel_set_speed(struct libinput_device *device, double speed)
{
    if (device->setPointerAccelerationReturnValue == 0) {
        device->pointerAcceleration = speed;
        return LIBINPUT_CONFIG_STATUS_SUCCESS;
    }
    return LIBINPUT_CONFIG_STATUS_INVALID;
}

enum libinput_config_status libinput_device_config_send_events_set_mode(struct libinput_device *device, uint32_t mode)
{
    if (device->setEnableModeReturnValue == 0) {
        device->enabled = (mode == LIBINPUT_CONFIG_SEND_EVENTS_ENABLED);
        return LIBINPUT_CONFIG_STATUS_SUCCESS;
    }
    return LIBINPUT_CONFIG_STATUS_INVALID;
}


enum libinput_event_type libinput_event_get_type(struct libinput_event *event)
{
    return event->type;
}

struct libinput_device *libinput_event_get_device(struct libinput_event *event)
{
    return event->device;
}

void libinput_event_destroy(struct libinput_event *event)
{
    delete event;
}

struct libinput_event_keyboard *libinput_event_get_keyboard_event(struct libinput_event *event)
{
    if (event->type == LIBINPUT_EVENT_KEYBOARD_KEY) {
        return reinterpret_cast<libinput_event_keyboard *>(event);
    }
    return nullptr;
}

struct libinput_event_pointer *libinput_event_get_pointer_event(struct libinput_event *event)
{
    if (event->type == LIBINPUT_EVENT_POINTER_MOTION ||
        event->type == LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE ||
        event->type == LIBINPUT_EVENT_POINTER_BUTTON ||
        event->type == LIBINPUT_EVENT_POINTER_AXIS) {
        return reinterpret_cast<libinput_event_pointer *>(event);
    }
    return nullptr;
}

struct libinput_event_touch *libinput_event_get_touch_event(struct libinput_event *event)
{
    if (event->type == LIBINPUT_EVENT_TOUCH_DOWN ||
        event->type == LIBINPUT_EVENT_TOUCH_UP ||
        event->type == LIBINPUT_EVENT_TOUCH_MOTION ||
        event->type == LIBINPUT_EVENT_TOUCH_CANCEL ||
        event->type == LIBINPUT_EVENT_TOUCH_FRAME) {
        return reinterpret_cast<libinput_event_touch *>(event);
    }
    return nullptr;
}

struct libinput_event_gesture *libinput_event_get_gesture_event(struct libinput_event *event)
{
    if (event->type == LIBINPUT_EVENT_GESTURE_PINCH_BEGIN ||
        event->type == LIBINPUT_EVENT_GESTURE_PINCH_UPDATE ||
        event->type == LIBINPUT_EVENT_GESTURE_PINCH_END ||
        event->type == LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN ||
        event->type == LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE ||
        event->type == LIBINPUT_EVENT_GESTURE_SWIPE_END) {
        return reinterpret_cast<libinput_event_gesture *>(event);
    }
    return nullptr;
}

int libinput_event_gesture_get_cancelled(struct libinput_event_gesture *event)
{
    if (event->type == LIBINPUT_EVENT_GESTURE_PINCH_END || event->type == LIBINPUT_EVENT_GESTURE_SWIPE_END) {
        return event->cancelled;
    }
    return 0;
}

uint32_t libinput_event_gesture_get_time(struct libinput_event_gesture *event)
{
    return event->time;
}

int libinput_event_gesture_get_finger_count(struct libinput_event_gesture *event)
{
    return event->fingerCount;
}

double libinput_event_gesture_get_dx(struct libinput_event_gesture *event)
{
    if (event->type == LIBINPUT_EVENT_GESTURE_PINCH_UPDATE || event->type == LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE) {
        return event->delta.width();
    }
    return 0.0;
}

double libinput_event_gesture_get_dy(struct libinput_event_gesture *event)
{
    if (event->type == LIBINPUT_EVENT_GESTURE_PINCH_UPDATE || event->type == LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE) {
        return event->delta.height();
    }
    return 0.0;
}

double libinput_event_gesture_get_scale(struct libinput_event_gesture *event)
{
    switch (event->type) {
    case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
        return 1.0;
    case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
    case LIBINPUT_EVENT_GESTURE_PINCH_END:
        return event->scale;
    default:
        return 0.0;
    }
}

double libinput_event_gesture_get_angle_delta(struct libinput_event_gesture *event)
{
    if (event->type == LIBINPUT_EVENT_GESTURE_PINCH_UPDATE) {
        return event->angleDelta;
    }
    return 0.0;
}

uint32_t libinput_event_keyboard_get_key(struct libinput_event_keyboard *event)
{
    return event->key;
}

enum libinput_key_state libinput_event_keyboard_get_key_state(struct libinput_event_keyboard *event)
{
    return event->state;
}

uint32_t libinput_event_keyboard_get_time(struct libinput_event_keyboard *event)
{
    return event->time;
}

double libinput_event_pointer_get_absolute_x(struct libinput_event_pointer *event)
{
    return event->absolutePos.x();
}

double libinput_event_pointer_get_absolute_y(struct libinput_event_pointer *event)
{
    return event->absolutePos.y();
}

double libinput_event_pointer_get_absolute_x_transformed(struct libinput_event_pointer *event, uint32_t width)
{
    double deviceWidth = 0.0;
    double deviceHeight = 0.0;
    libinput_device_get_size(event->device, &deviceWidth, &deviceHeight);
    return event->absolutePos.x() / deviceWidth * width;
}

double libinput_event_pointer_get_absolute_y_transformed(struct libinput_event_pointer *event, uint32_t height)
{
    double deviceWidth = 0.0;
    double deviceHeight = 0.0;
    libinput_device_get_size(event->device, &deviceWidth, &deviceHeight);
    return event->absolutePos.y() / deviceHeight * height;
}

double libinput_event_pointer_get_dx(struct libinput_event_pointer *event)
{
    return event->delta.width();
}

double libinput_event_pointer_get_dy(struct libinput_event_pointer *event)
{
    return event->delta.height();
}

uint32_t libinput_event_pointer_get_time(struct libinput_event_pointer *event)
{
    return event->time;
}

uint32_t libinput_event_pointer_get_button(struct libinput_event_pointer *event)
{
    return event->button;
}

enum libinput_button_state libinput_event_pointer_get_button_state(struct libinput_event_pointer *event)
{
    return event->buttonState;
}

int libinput_event_pointer_has_axis(struct libinput_event_pointer *event, enum libinput_pointer_axis axis)
{
    if (axis == LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL) {
        return event->verticalAxis;
    } else {
        return event->horizontalAxis;
    }
}

double libinput_event_pointer_get_axis_value(struct libinput_event_pointer *event, enum libinput_pointer_axis axis)
{
    if (axis == LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL) {
        return event->verticalAxisValue;
    } else {
        return event->horizontalAxisValue;
    }
}

uint32_t libinput_event_touch_get_time(struct libinput_event_touch *event)
{
    return event->time;
}

double libinput_event_touch_get_x(struct libinput_event_touch *event)
{
    return event->absolutePos.x();
}

double libinput_event_touch_get_y(struct libinput_event_touch *event)
{
    return event->absolutePos.y();
}

double libinput_event_touch_get_x_transformed(struct libinput_event_touch *event, uint32_t width)
{
    double deviceWidth = 0.0;
    double deviceHeight = 0.0;
    libinput_device_get_size(event->device, &deviceWidth, &deviceHeight);
    return event->absolutePos.x() / deviceWidth * width;
}

double libinput_event_touch_get_y_transformed(struct libinput_event_touch *event, uint32_t height)
{
    double deviceWidth = 0.0;
    double deviceHeight = 0.0;
    libinput_device_get_size(event->device, &deviceWidth, &deviceHeight);
    return event->absolutePos.y() / deviceHeight * height;
}

int32_t libinput_event_touch_get_slot(struct libinput_event_touch *event)
{
    return event->slot;
}

struct libinput *libinput_udev_create_context(const struct libinput_interface *interface, void *user_data, struct udev *udev)
{
    if (!udev) {
        return nullptr;
    }
    Q_UNUSED(interface)
    Q_UNUSED(user_data)
    return new libinput;
}

void libinput_log_set_priority(struct libinput *libinput, enum libinput_log_priority priority)
{
    Q_UNUSED(libinput)
    Q_UNUSED(priority)
}

void libinput_log_set_handler(struct libinput *libinput, libinput_log_handler log_handler)
{
    Q_UNUSED(libinput)
    Q_UNUSED(log_handler)
}

struct libinput *libinput_unref(struct libinput *libinput)
{
    libinput->refCount--;
    if (libinput->refCount == 0) {
        delete libinput;
        return nullptr;
    }
    return libinput;
}

int libinput_udev_assign_seat(struct libinput *libinput, const char *seat_id)
{
    if (libinput->assignSeatRetVal == 0) {
        libinput->seat = QByteArray(seat_id);
    }
    return libinput->assignSeatRetVal;
}

int libinput_get_fd(struct libinput *libinput)
{
    Q_UNUSED(libinput)
    return -1;
}

int libinput_dispatch(struct libinput *libinput)
{
    Q_UNUSED(libinput)
    return 0;
}

struct libinput_event *libinput_get_event(struct libinput *libinput)
{
    Q_UNUSED(libinput)
    return nullptr;
}

void libinput_suspend(struct libinput *libinput)
{
    Q_UNUSED(libinput)
}

int libinput_resume(struct libinput *libinput)
{
    Q_UNUSED(libinput)
    return 0;
}
