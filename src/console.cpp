/*
    src/Console.cpp -- Simple console

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/window.h>
#include <nanogui/screen.h>
#include <nanogui/layout.h>
#include <nanogui/console.h>
#include <nanogui/opengl.h>
#include <nanogui/theme.h>
#include <nanogui/serializer/core.h>
#include <regex>
#include <nanogui_resources.h>
#include <iostream>
#include <algorithm>

NAMESPACE_BEGIN(nanogui)

Console::Console(Widget *parent):
    Widget(parent),
    mCommitted(true),
    mScroll(0.0f),
    current_index(99),
    mInit(true),
      mCaret(this),
      mMousePos(Vector2i(-1,-1)),
      mMouseDownPos(Vector2i(-1,-1)),
      mMouseDragPos(Vector2i(-1,-1)),
      mMouseDownModifier(0)
{
    //mText = {"The very first entry","Entry 0", "Entry 1", "Entry 2",
    //    "Entry 3","Entry 4","Entry 5","Entry 6",
    //    "Entry 7 On a clear sunny day, the sky above us looks bright blue. "
    //        "In the evening, the sunset puts on a brilliant show of reds, pinks and oranges.End.",
    //    "Entry 8","Entry 9", "Entry 10", "The very last entry"};
    for(int i=0;i<5;++i){

        mHistory.push_back("text string "+std::to_string(i+1)+" of 100: " 
        + "On a clear sunny day, the sky above us looks bright blue. " +
            "In the evening, the sunset puts on a brilliant show of reds, pinks and oranges.End.");
    }

    mCommand.emplace_back("");

    mTopPart = 1;
    mTopRow = 0;
    mTopSubrow = 0;

    std::cout << "mPos = " << mPos.x() << "," << mPos.y() << std::endl;
}

//void Console::performLayout(NVGcontext *ctx) {
//    Widget::performLayout(ctx);
//}

Vector2i Console::preferredSize(NVGcontext * /*ctx*/) const {
    return Vector2i(100, 100);
}

void Console::draw(NVGcontext* ctx){
    Widget::draw(ctx);  

    //Screen *sc = dynamic_cast<Screen *>(this->window()->parent());

    // Draw rounded rectangel around text area ////////////////////
    NVGpaint paint = nvgBoxGradient(
        ctx, mPos.x() + 1, mPos.y() + 1,
        mSize.x()-2, mSize.y(), 3, 4, Color(0, 32), Color(0, 92));
    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), 3);
    nvgFillPaint(ctx, paint);
    nvgFill(ctx);
    ///////////////////////////////////////////////////////////////
    


    nvgFontSize(ctx, 18.0f);
    nvgFontFace(ctx, "sans");
    nvgTextAlign(ctx, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
    float x, y, linew, lineh;
    nvgTextMetrics(ctx, nullptr, nullptr, &lineh);
    x = mPos.x();
    y = mPos.y();
    linew = mSize.x();
    
    mNumRows = mSize.y()/lineh ; // make sure that the size of console is updated
    //setSize(Vector2i(mSize.x(), nrows*lineh)); // this code does not work. It
    //changes size each frame. Move from draw into one time function.
    
    // init console output 
    if(mInit) {
        initBuffer(ctx, linew);
        mInit = false;
    }
    //typedef std::deque<std::string> Buffer_t;

    //temp solution: when buffer is not full. Move this to updateFunction.
    for(auto it = mBuffer.begin();it!=mBuffer.end();++it) {

        nvgBeginPath(ctx);
        nvgFillColor(ctx, nvgRGBA(255,255,255,16));
        nvgRect(ctx, x, y, linew, lineh);
        nvgFill(ctx);

        nvgFillColor(ctx, nvgRGBA(255,255,255,255));
        nvgText(ctx, x, y, it->c_str(), nullptr);
        
        y += lineh;
    }
   
    updateCursor(ctx, lineh);
    //update cursor and draw
    //mCaret.onClick(ctx, lineh, mMouseDownPos - mPos);
    mCaret.draw(ctx, lineh, mPos);
}

bool Caret::isVisible() {
    return mPos.x() > -1 && mPos.y() > -1;
}

void Caret::draw(NVGcontext *ctx, float lineh, Vector2i offset) {
    
    //std::cout << "Cursor("<< mCursorPos.x()<< ","<< mCursorPos.y() <<")" << std::endl;

    if(isVisible()) {
        //float caretx = cursorIndex2Position(mCursorPos.x(), textBound[2], glyphs, nglyphs);
        //mPosition = getCursorPosition(ctx, mCursorPos, lineh);

        // draw cursor
        nvgBeginPath(ctx);
        nvgMoveTo(ctx, offset.x() + mPos.x(), offset.y()+ mPos.y()  );
        nvgLineTo(ctx, offset.x() + mPos.x(), offset.y()+ mPos.y()+ lineh );
        
        nvgStrokeColor(ctx, nvgRGBA(255, 192, 0, 255));
        nvgStrokeWidth(ctx, 1.0f);
        nvgStroke(ctx);
    }

    if(mSelectionState) {
        // draw selection
        int linew = mConsole->mSize.x();
        nvgBeginPath(ctx);
        nvgFillColor(ctx, nvgRGBA(255, 255, 255, 80));
        if(mIdx.y() == mSelectionIdx.y()) {
            int x = std::min(mPos.x(),mSelectionPos.x());
            x += offset.x();
            nvgRect(ctx, x, offset.y()+ mPos.y(), std::abs(mSelectionPos.x()-mPos.x()), lineh);
        } else if(mIdx.y() > mSelectionIdx.y()) {
            nvgRect(ctx, offset.x()+mSelectionPos.x(), offset.y()+ mSelectionPos.y(), 
                    linew-mSelectionPos.x(), lineh);
            if(mIdx.y()-mSelectionIdx.y()>1)
                nvgRect(ctx, offset.x(), offset.y()+ mSelectionPos.y()+lineh, 
                    linew, mPos.y()-mSelectionPos.y()-lineh);
            nvgRect(ctx, offset.x(), offset.y()+ mPos.y(), mPos.x(), lineh);
        } else {
            nvgRect(ctx, offset.x(), offset.y()+ mSelectionPos.y(), 
                    mSelectionPos.x(), lineh);
            if(mSelectionIdx.y()-mIdx.y()>1)
                nvgRect(ctx, offset.x(), offset.y()+ mPos.y()+lineh, 
                    linew, mSelectionPos.y()-mPos.y()-lineh);
            nvgRect(ctx, offset.x()+mPos.x(), offset.y()+ mPos.y(), linew-mPos.x(), lineh);
        }
        nvgFill(ctx);
    }
}

void Caret::onClick(NVGcontext *ctx, float lineh, Vector2i clickPos){
    if (clickPos.x() > -1) {
        mIdx.y() = (int) clickPos.y()/lineh;
        //std::cout << "mPos = " << mConsole->mPos.x() << "," << mConsole->mPos.y() << std::endl;

        if(mIdx.y() < (int)mConsole->mBuffer.size()){
            //std::cout << mBuffer.at(mCursorPosY)<< std::endl;
            double time = glfwGetTime();
            if (time - mLastClick < 0.25) {
                /* Double-click: select a line */
                int linew = mConsole->mSize.x();
                mSelectionIdx = Vector2i(0,(int) clickPos.y()/lineh); 
                mSelectionPos = Vector2i(0,mSelectionIdx.y()*lineh);
                mIdx = Vector2i(mConsole->mBuffer[mSelectionIdx.y()].size(), mSelectionIdx.y());
                mPos = Vector2i(linew, mSelectionPos.y());
                mSelectionState = true;
                std::cout << "Sel : " << mSelectionPos.x() << "," << mSelectionPos.y() << std::endl;
                std::cout << "Pos : " << mPos.x() << "," << mPos.y() << std::endl;
            }
            else { 
                const int maxGlyphs = 1024;
                NVGglyphPosition glyphs[maxGlyphs];
                const std::string& textLine = mConsole->mBuffer[mIdx.y()];
                float textBound[4];

                nvgTextBounds(ctx, 0, 0, textLine.c_str(), nullptr, textBound);
                int nglyphs = nvgTextGlyphPositions(ctx, 0, 0, textLine.c_str(),
                        nullptr, glyphs, maxGlyphs);

                mIdx.x() = position2CursorIndex(clickPos.x(), textBound[2], glyphs, nglyphs);
         
                //update carete position now
                mPos.y() = mIdx.y() * lineh;
                mPos.x() = cursorIndex2Position(mIdx.x(), textBound[2], glyphs, nglyphs);
            }
            mLastClick = time;
        }
    }
}

void Caret::updatePosFromIdx(NVGcontext *ctx) {
    
    float lineh;
    nvgTextMetrics(ctx, nullptr, nullptr, &lineh);
    const int maxGlyphs = 1024;
    NVGglyphPosition glyphs[maxGlyphs];
    const std::string& textLine = mConsole->mBuffer[mIdx.y()];
    float textBound[4];

    nvgTextBounds(ctx, 0, 0, textLine.c_str(), nullptr, textBound);
    int nglyphs = nvgTextGlyphPositions(ctx, 0, 0, textLine.c_str(),
            nullptr, glyphs, maxGlyphs);

    //update carete position now
    mPos.y() = mIdx.y() * lineh;
    mPos.x() = cursorIndex2Position(mIdx.x(), textBound[2], glyphs, nglyphs);
}
int Caret::position2CursorIndex(float posx, float lastx,
                                  const NVGglyphPosition *glyphs, int size) {
    int mCursorId = 0;
    float caretx = glyphs[mCursorId].x;
    for (int j = 1; j < size; j++) {
        if (std::abs(caretx - posx) > std::abs(glyphs[j].x - posx)) {
            mCursorId = j;
            caretx = glyphs[mCursorId].x;
        }
    }
    if (std::abs(caretx - posx) > std::abs(lastx - posx))
        mCursorId = size;

    return mCursorId;
}

float Caret::cursorIndex2Position(int index, float lastx,
                                    const NVGglyphPosition *glyphs, int size) {
    float pos = 0;
    if (index == size)
        pos = lastx; // last character
    else
        pos = glyphs[index].x;

    return pos;
}

void Caret::moveLeft() {
    if(mIdx.x() > 0) {
        --mIdx.x();
        Screen *sc = dynamic_cast<Screen *>(mConsole->window()->parent());
        updatePosFromIdx(sc->nvgContext());
    }
}

void Caret::moveRight() {
    const std::string& textLine = mConsole->mBuffer[mIdx.y()];
    if(mIdx.x() < (int) textLine.size()) {
        ++mIdx.x();
        Screen *sc = dynamic_cast<Screen *>(mConsole->window()->parent());
        updatePosFromIdx(sc->nvgContext());
    }
}

void Caret::moveUp() {
    if(mIdx.y() > 0) {
        --mIdx.y();
        int textSize = (int) mConsole->mBuffer[mIdx.y()].size();
        if(mIdx.x() >= textSize) 
            mIdx.x() = textSize;
        Screen *sc = dynamic_cast<Screen *>(mConsole->window()->parent());
        updatePosFromIdx(sc->nvgContext());
    }
}

void Caret::moveDown() {
    if(mIdx.y() < (int) mConsole->mBuffer.size()) {
        ++mIdx.y();
        int textSize = (int) mConsole->mBuffer[mIdx.y()].size();
        if(mIdx.x() >= textSize) 
            mIdx.x() = textSize;
        Screen *sc = dynamic_cast<Screen *>(mConsole->window()->parent());
        updatePosFromIdx(sc->nvgContext());
    }
}

void Console::updateCursor(NVGcontext * ctx, float lineh) {
    
    // handle mouse cursor events
    if (mMouseDownPos.x() != -1) {
        if (mMouseDownModifier == GLFW_MOD_SHIFT)
            mCaret.startSelection();
        else 
            mCaret.resetSelection();

        mCaret.onClick(ctx, lineh, mMouseDownPos - mPos);

        mMouseDownPos = Vector2i(-1, -1);
    } else if (mMouseDragPos.x() != -1) {
        mCaret.startSelection();
        mCaret.onClick(ctx, lineh, mMouseDragPos - mPos);
    } 
//    else {
//        // set cursor to last character
//        if (mCursorPos.x() == -2)
//            //mCursorPos = size; //update
//            //std::cout << "**********    -2 *************" << std::endl;
//            ;
//    }

//    if (mCursorPos == mSelectionPos)
//        mSelectionPos = Vector2i(-1,-1);
}


bool Console::mouseMotionEvent(const Vector2i &p, const Vector2i & /* rel */,
                               int /* button */, int /* modifiers */) {
    //std::cout << "mouseMotionEvent " << p  << " " << rel << 
    //        button << " " << modifiers <<  std::endl;
    mMousePos = p;

    setCursor(Cursor::IBeam);

    if (focused()) {
        return true;
    }
    return false;

}

bool Console::mouseDragEvent(const Vector2i &p, const Vector2i &/* rel*/,
                             int /* button */, int /* modifiers */) {
    //std::cout << "mouseDragEvent " << p << " " << rel << " " <<
    //     button << modifiers << std::endl;
    mMousePos = p;
    mMouseDragPos = p;

    if (focused()) {
        return true;
    }
    return false;
}

bool Console::mouseButtonEvent(const Vector2i &p, int button, 
                               bool down, int modifiers) {
    //std::cout << "mouseButtonEvent " << p.x()<<","<<p.y() << " " << button << " " <<
    //    down << " " << modifiers << std::endl;
    
    if (button == GLFW_MOUSE_BUTTON_1 && down && !mFocused) {
        requestFocus();
    }

    if (focused()) {
        if (down) {
            mMouseDownPos = p;
            mMouseDownModifier = modifiers;

        } else {
            mMouseDownPos = Vector2i(-1, -1);
            mMouseDragPos = Vector2i(-1, -1);
        }
        return true;
    }
    return false;
}

//bool Console::focusEvent(bool focused) {
//    Widget::focusEvent(focused);
//
//        if (focused) {
//            mCursorPos = 0;
//        } else {
//            mCursorPos = -1;
//            mSelectionPos = -1;
//        }
//
//    return true;
//}

void Console::initBuffer(NVGcontext * ctx, float linew) {

    int currPart = mTopPart;
    int currRow  = mTopRow;
    int currSubrow = mTopSubrow;

//    const char *start, *end;
    int maxrows = 128;
    NVGtextRow rows[maxrows];
    
    //std::cout << "mNumRows = " << mNumRows << std::endl; 
    
    int i=0;
    for(;i<mNumRows && currRow<(int)mHistory.size();) {
        if(currPart) { // if history part
            if(currSubrow < 0) {
                //BufferRow bRow(mHistory[currRow], currRow;
                mBuffer.emplace_back(mHistory[currRow]);
                ++i;
            }
            else {
                const char *text = mHistory[currRow].c_str();
                int nsplits = nvgTextBreakLines(ctx, text, nullptr, linew, rows, maxrows);
                
                while(i<mNumRows && currSubrow < nsplits) {
                    NVGtextRow *subrow = &rows[currSubrow];
                    std::string temp(subrow->start, subrow->end);
                    mBuffer.emplace_back(temp);
                    ++i;
                    ++currSubrow;
                }
            }
            ++currRow;
            currSubrow = 0;
        }
    }
    
    if(i<mNumRows){
        if (mCommand[0] == "")
            mBuffer.emplace_back("$");
    }
}

bool Console::scrollEvent(const Vector2i &/* p */, const Vector2f &rel) {
    float scrollAmount = rel.y() * (mSize.y() / 20.0f);
    scrollAmount = rel.y()*5;
    //float scrollh = height() *
    //    std::min(1.0f, height() / (float)mChildPreferredHeight);

    //mScroll = std::max((float) 0.0f, std::min((float) 1.0f,
    //        mScroll - scrollAmount / (float)(mSize.y() - 8 - scrollh)));
    //std::cout << "Scroll amount = " << scrollAmount << std::endl;
    mScroll = scrollAmount;

    return true;
}

bool Console::keyboardEvent(int key, int /* scancode */, int action, int modifiers) {
    if (focused()) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            if (key == GLFW_KEY_LEFT) {
                if (modifiers == GLFW_MOD_SHIFT)
                    mCaret.startSelection();
                else
                    mCaret.resetSelection();

                mCaret.moveLeft();
            } else if (key == GLFW_KEY_RIGHT) {
                if (modifiers == GLFW_MOD_SHIFT)
                    mCaret.startSelection();
                else
                    mCaret.resetSelection();

                mCaret.moveRight();
            } else if (key == GLFW_KEY_UP) {
                if (modifiers == GLFW_MOD_SHIFT)
                    mCaret.startSelection();
                else
                    mCaret.resetSelection();

                mCaret.moveUp();
            } else if (key == GLFW_KEY_DOWN) {
                if (modifiers == GLFW_MOD_SHIFT)
                    mCaret.startSelection();
                else
                    mCaret.resetSelection();

                mCaret.moveDown();
//            } else if (key == GLFW_KEY_HOME) {
//                if (modifiers == GLFW_MOD_SHIFT) {
//                    if (mSelectionPos == -1)
//                        mSelectionPos = mCursorPos;
//                } else {
//                    mSelectionPos = -1;
//                }
//
//                mCursorPos = 0;
//            } else if (key == GLFW_KEY_END) {
//                if (modifiers == GLFW_MOD_SHIFT) {
//                    if (mSelectionPos == -1)
//                        mSelectionPos = mCursorPos;
//                } else {
//                    mSelectionPos = -1;
//                }
//
//                mCursorPos = (int) mValueTemp.size();
//            } else if (key == GLFW_KEY_BACKSPACE) {
//                if (!deleteSelection()) {
//                    if (mCursorPos > 0) {
//                        mValueTemp.erase(mValueTemp.begin() + mCursorPos - 1);
//                        mCursorPos--;
//                    }
//                }
//            } else if (key == GLFW_KEY_DELETE) {
//                if (!deleteSelection()) {
//                    if (mCursorPos < (int) mValueTemp.length())
//                        mValueTemp.erase(mValueTemp.begin() + mCursorPos);
//                }
            } else if (key == GLFW_KEY_ENTER) {
                if (!mCommitted)
                    focusEvent(false);
//            } else if (key == GLFW_KEY_A && modifiers == SYSTEM_COMMAND_MOD) {
//                mCursorPos = (int) mValueTemp.length();
//                mSelectionPos = 0;
//            } else if (key == GLFW_KEY_X && modifiers == SYSTEM_COMMAND_MOD) {
//                copySelection();
//                deleteSelection();
//            } else if (key == GLFW_KEY_C && modifiers == SYSTEM_COMMAND_MOD) {
//                copySelection();
//            } else if (key == GLFW_KEY_V && modifiers == SYSTEM_COMMAND_MOD) {
//                deleteSelection();
//                pasteFromClipboard();
            }

        }

        return true;
    }

    return false;
}

bool Console::keyboardCharacterEvent(unsigned int codepoint) {
    if (focused()) {
        std::ostringstream convert;
        convert << (char) codepoint;

//        deleteSelection();
//        mValueTemp.insert(mCursorPos, convert.str());
//        mCursorPos++;
//
        return true;
    }

    return false;
}

NAMESPACE_END(nanogui)
