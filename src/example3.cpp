/*
    src/example1.cpp -- C++ version of an example application that shows
    how to use the various widget classes. For a Python implementation, see
    '../python/example1.py'.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <nanogui/console.h>
#if defined(_WIN32)
#include <windows.h>
#endif
#include <nanogui/glutil.h>
#include <iostream>
#include <string>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::to_string;

class ExampleApplication : public nanogui::Screen {
public:
    ExampleApplication() : nanogui::Screen(Eigen::Vector2i(1024, 768), "NanoGUI Test") {
        using namespace nanogui;

        Window *window = new Window(this, "Button demo");
        window->setPosition(Vector2i(15, 15));
        window->setLayout(new GroupLayout());
        mProgress = new ProgressBar(window);

              
        // Console example 
        //
        window = new Window(this, "Simple Console");
        window->setPosition(Vector2i(145, 15));
        window->setLayout(new GroupLayout());

        Console *console = new Console(window);
//        console->setFixedSize(Vector2i(400, 200));
//        console->setValue("Some initial text \n with line break");
        console->setFontSize(20);
//        console->setEditable(true);
//        console->setAlignment(Console::Alignment::Left);


        performLayout();

   }

    ~ExampleApplication() {
    }

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            setVisible(false);
            return true;
        }
        return false;
    }

    virtual void draw(NVGcontext *ctx) {
        /* Animate the scrollbar */
        mProgress->setValue(std::fmod((float) glfwGetTime() / 10, 1.0f));

        /* Draw the user interface */
        Screen::draw(ctx);
    }

    virtual void drawContents() {
    }

private:
    nanogui::ProgressBar *mProgress;
    nanogui::GLShader mShader;
};

int main(int /* argc */, char ** /* argv */) {
    try {
        nanogui::init();

        {
            nanogui::ref<ExampleApplication> app = new ExampleApplication();
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop();
        }

        nanogui::shutdown();
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(_WIN32)
            MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
            std::cerr << error_msg << endl;
        #endif
        return -1;
    }

    return 0;
}
