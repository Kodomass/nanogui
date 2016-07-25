/*
    nanogui/console.h -- Simple console.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <nanogui/compat.h>
#include <nanogui/widget.h>
#include <nanogui/textbox.h>
#include <sstream>
#include <deque>

NAMESPACE_BEGIN(nanogui)

class Console;

class Caret {
public:
    Caret(Console *console) :
        mConsole(console),
        mPos(Vector2i(-1,-1)),
        mSelectionPos(Vector2i(-1,-1)),
        mIdx(Vector2i(-1,-1)),
        mSelectionIdx(Vector2i(-1,-1)),
        mSelectionState(false),
        mLastClick(0) {}

    void draw(NVGcontext* ctx, float h, Vector2i origin);
    bool isVisible();
   
    void onClick(NVGcontext *ctx, float h, Vector2i clickPos);
    int position2CursorIndex(float posx, float lastx,
                                  const NVGglyphPosition *glyphs, int size);
    float cursorIndex2Position(int index, float lastx,
                                    const NVGglyphPosition *glyphs, int size);
    void startSelection() {
        if(!mSelectionState){
            mSelectionPos = mPos;
            mSelectionIdx = mIdx;
            mSelectionState = true;
        }
    }

    void resetSelection(){
        mSelectionPos = Vector2i(-1,-1);
        mSelectionIdx = Vector2i(-1,-1);
        mSelectionState = false;
    }
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    void updatePosFromIdx(NVGcontext *ctx);
private:
 
    Console *mConsole;
    
    // Position on screen
    Vector2i mPos;
    Vector2i mSelectionPos;
    
    // Index in text
    Vector2i mIdx;
    Vector2i mSelectionIdx;
    
    bool  mSelectionState;
    double mLastClick;
};

class NANOGUI_EXPORT Console : public Widget {

public:
    Console(Widget *parent);
    
    void initBuffer(NVGcontext * ctx, float linew);

    //virtual void performLayout(NVGcontext *ctx);
    virtual Vector2i preferredSize(NVGcontext *ctx) const override;
    virtual void draw(NVGcontext* ctx) override;
    virtual bool scrollEvent(const Vector2i &/* p */, const Vector2f &rel) override;

    void updateCursor(NVGcontext *ctx, float lineh);
    //Vector2i getCursorIndex(NVGcontext *ctx, Vector2i pos, float lineh);
    //Vector2i getCursorPosition(NVGcontext *ctx, Vector2i cursorId, float lineh);
    virtual bool mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers) override;
    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) override;
    virtual bool mouseDragEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers) override;
    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) override;
    virtual bool keyboardCharacterEvent(unsigned int codepoint) override;
    //int position2CursorIndex(float posx, float lastx,
    //                              const NVGglyphPosition *glyphs, int size);
    //float cursorIndex2Position(int index, float lastx,
    //                                const NVGglyphPosition *glyphs, int size);
    //bool Console::focusEvent(bool focused);
    void drawCursor(NVGcontext* ctx, Vector2i origin);
    friend class Caret; 
protected:

    bool mCommitted;
    float mScroll;
    int current_index;
   
    // total number of rows in the console
    int mNumRows;

struct BufferRow{
    std::string value;
    float width;
    int index;
    int subindex;
};

    std::deque<std::string>  mBuffer;
    //std::deque<BufferRow>  mBuffer;
    std::vector<std::string> mHistory;
    std::vector<std::string> mCommand;
    std::vector<std::string> mCommandHistory;

    int mTopPart;
    int mTopRow;
    int mTopSubrow;

    int mBotPart;
    int mBotRow;
    int mBotSubrow;

    bool mInit;
    
    //bool mEditable;
    Caret mCaret;
    
    Vector2i mMousePos;
    Vector2i mMouseDownPos;
    Vector2i mMouseDragPos;
    
    int mMouseDownModifier;
};

NAMESPACE_END(nanogui)
