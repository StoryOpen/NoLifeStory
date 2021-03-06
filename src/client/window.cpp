//////////////////////////////////////////////////////////////////////////////
// NoLifeClient - Part of the NoLifeStory project                           //
// Copyright © 2014 Peter Atashian                                          //
//                                                                          //
// This program is free software: you can redistribute it and/or modify     //
// it under the terms of the GNU Affero General Public License as           //
// published by the Free Software Foundation, either version 3 of the       //
// License, or (at your option) any later version.                          //
//                                                                          //
// This program is distributed in the hope that it will be useful,          //
// but WITHOUT ANY WARRANTY; without even the implied warranty of           //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            //
// GNU Affero General Public License for more details.                      //
//                                                                          //
// You should have received a copy of the GNU Affero General Public License //
// along with this program.  If not, see <http://www.gnu.org/licenses/>.    //
//////////////////////////////////////////////////////////////////////////////

#include "window.hpp"
#include "config.hpp"
#include "time.hpp"
#include "game.hpp"
#include "view.hpp"
#include "sprite.hpp"
#include "map.hpp"
#include "sound.hpp"
#include "player.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <chrono>

namespace nl {
namespace window {
GLFWwindow * window = nullptr;
GLFWwindow * context = nullptr;
std::string title = "NoLifeStory";
std::chrono::steady_clock::time_point last_title;
// Callbacks for GLFW events
namespace callback {
void position(GLFWwindow *, int, int) {}
void resize(GLFWwindow *, int, int) {}
void closed(GLFWwindow *) { game::shut_down(); }
void refresh(GLFWwindow *) {}
void focus(GLFWwindow *, int) {
    // GL_TRUE focused
    // GL_FALSE unfocused
}
void minimize(GLFWwindow *, int) {
    // GL_TRUE minimized
    // GL_FALSE restored
}
void framebuffer(GLFWwindow *, int width, int height) {
    if (width && height) view::resize(width, height);
}
void mouse(GLFWwindow *, int, int, int) {}
void cursorpos(GLFWwindow *, double, double) {}
void cursorenter(GLFWwindow *, int) {}
void scroll(GLFWwindow *, double, double) {}
void key(GLFWwindow *, int key, int, int action, int mod) {
    switch (action) {
    case GLFW_REPEAT:
        switch (key) {
        case GLFW_KEY_LEFT_ALT:
        case GLFW_KEY_RIGHT_ALT: player::ch.pos.jump(); break;
        default: ;
        }
        break;
    case GLFW_PRESS:
        switch (key) {
        case GLFW_KEY_LEFT: player::ch.pos.left = true; break;
        case GLFW_KEY_RIGHT: player::ch.pos.right = true; break;
        case GLFW_KEY_UP: player::ch.pos.up = true; break;
        case GLFW_KEY_DOWN: player::ch.pos.down = true; break;
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_RIGHT_SHIFT: player::mouse_fly = true; break;
        case GLFW_KEY_LEFT_ALT:
        case GLFW_KEY_RIGHT_ALT: player::ch.pos.jump(); break;
        case GLFW_KEY_ENTER:
            if (mod & GLFW_MOD_ALT) recreate(!config::fullscreen);
            break;
        case GLFW_KEY_M: map::load_random(); break;
        case GLFW_KEY_R:
            config::rave = !config::rave;
            music::play();
            break;
        default: ;
        }
        break;
    default: ;
    }
}
void character(GLFWwindow *, unsigned int) {}
}
// Once GLFW supports toggling fullscreen without recreating, change this method
void recreate(bool fullscreen) {
    sprite::flush();
    if (window) glfwDestroyWindow(window);
    config::fullscreen = fullscreen;
    if (config::fullscreen)
        window = glfwCreateWindow(config::fullscreen_width, config::fullscreen_height,
                                  title.c_str(), glfwGetPrimaryMonitor(), context);
    else
        window = glfwCreateWindow(config::window_width, config::window_height, title.c_str(),
                                  nullptr, context);
    glfwMakeContextCurrent(window);
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    callback::framebuffer(window, w, h);
    glfwSwapInterval(config::vsync ? 1 : 0);
    glfwSetFramebufferSizeCallback(window, callback::framebuffer);
    glfwSetWindowCloseCallback(window, callback::closed);
    glfwSetWindowFocusCallback(window, callback::focus);
    glfwSetWindowIconifyCallback(window, callback::minimize);
    glfwSetWindowPosCallback(window, callback::position);
    glfwSetWindowRefreshCallback(window, callback::refresh);
    glfwSetWindowSizeCallback(window, callback::resize);
    glfwSetKeyCallback(window, callback::key);
    last_title = std::chrono::steady_clock::now();
}
void init() {
    if (glfwInit() != GL_TRUE) throw std::runtime_error("Failed to initialize GLFW");
    glfwWindowHint(GLFW_STENCIL_BITS, 0);
    glfwWindowHint(GLFW_DEPTH_BITS, 0);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_NO_RESET_NOTIFICATION);
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, 1);
    // We have to make an invisible context so we don't have to recreate everything whenever
    // fullscreen is toggled
    // Once GLFW supports fullscreen toggling better, this can be removed.
    context = glfwCreateWindow(1, 1, "", nullptr, nullptr);
    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
    glfwMakeContextCurrent(context);
    auto err = glewInit();
    switch (err) {
    case GLEW_OK: break;
    case GLEW_ERROR_NO_GL_VERSION:
        throw std::runtime_error(
            "Your OpenGL does not exist. Please update your drivers and/or buy a new GPU.");
    case GLEW_ERROR_GL_VERSION_10_ONLY:
        throw std::runtime_error(
            "Your OpenGL is out of date. Please update your drivers and/or buy a new GPU.");
    case GLEW_ERROR_GLX_VERSION_11_ONLY:
        throw std::runtime_error("Your GLX is out of date. Please update your X window system.");
    default: throw std::runtime_error("Unknown GLEW error code: " + std::to_string(err));
    }
    if (!GLEW_ARB_texture_non_power_of_two || !GLEW_VERSION_1_5)
        throw std::runtime_error(
            "Your OpenGL is out of date. Please update your drivers and/or buy a new GPU.");
}
void check_errors() {
    auto err = glGetError();
    switch (err) {
    case GL_NO_ERROR: break;
    case GL_INVALID_ENUM: throw std::runtime_error("Invalid enum");
    case GL_INVALID_VALUE: throw std::runtime_error("Invalid value");
    case GL_INVALID_OPERATION: throw std::runtime_error("Invalid operation");
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        throw std::runtime_error("Invalid framebuffer operation");
    case GL_OUT_OF_MEMORY: throw std::runtime_error("Out of memory");
    default: throw std::runtime_error("Unknown OpenGL error code " + std::to_string(err));
    }
}
void update() {
    // Certain platforms exhibit terrible performance when updating the title too quickly
    if (!config::fullscreen) {
        auto now = std::chrono::steady_clock::now();
        if (now - last_title > std::chrono::milliseconds(250)) {
            last_title = now;
            glfwSetWindowTitle(window, ("NoLifeStory {fps = " + std::to_string(time::fps)
                                        + "; map = " + map::current_name + ";};").c_str());
        }
    }
    sprite::flush();
    check_errors();
    glfwPollEvents();
    glfwSwapBuffers(window);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}
void unload() { glfwTerminate(); }
std::pair<double, double> mouse_pos() {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    return {x, y};
}
bool get_key(int key) { return glfwGetKey(window, key) == GLFW_PRESS; }
}
}
