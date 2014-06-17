/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Robert Carr <robert.carr@canonical.com>
 */

#ifndef UBUNTU_APPLICATION_UI_WINDOW_MIRCLIENT_PRIV_H_
#define UBUNTU_APPLICATION_UI_WINDOW_MIRCLIENT_PRIV_H_

#include <ubuntu/application/ui/window.h>
#include <ubuntu/application/ui/window_state.h>
#include <mir_toolkit/mir_client_library.h>

#include <EGL/egl.h>

#include <stddef.h>

#include <functional>
#include <memory>
#include <string>

namespace ubuntu
{
namespace application
{
namespace mir
{
namespace client
{
class Instance;
class WindowProperties;

class Window
{
public:
    Window(Instance& instance, WindowProperties* window);
    ~Window() = default;

    UAUiWindow* as_u_window();
    static Window* from_u_window(UAUiWindow* u_window);
    
    EGLNativeWindowType get_native_type();

    UApplicationUiWindowState state() const;

    void hide();
    void show();
    void request_fullscreen();

    void get_size(uint32_t *width, uint32_t *height);

    // user as in "platform-api user"
    UAUiWindowEventCb get_user_callback() { return user_event_callback;}
    void *get_user_callback_context() { return user_event_callback_context; }

protected:
    Window(Window const&) = delete;
    Window& operator=(Window const&) = delete;

private:
    void set_state(const UApplicationUiWindowState);

    Instance& instance;

    typedef std::unique_ptr<WindowProperties, std::function<void(WindowProperties*)>> WindowPropertiesPtr;
    typedef std::unique_ptr<MirSurface, std::function<void(MirSurface*)>> SurfacePtr;

    WindowPropertiesPtr window_properties;
    SurfacePtr surface;

    UAUiWindowEventCb user_event_callback;
    void *user_event_callback_context;

    UApplicationUiWindowState state_before_hiding;
};
    
}
}
}
}

#endif // UBUNTU_APPLICATION_UI_WINDOW_MIRCLIENT_PRIV_H_
