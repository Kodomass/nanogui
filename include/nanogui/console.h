/*
    nanogui/Console.h -- Simple console.

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

NAMESPACE_BEGIN(nanogui)

class ConsoleRow : public TextBox {

public:

    ConsoleRow(Widget *parent, int id, const std::string &value = "") :  TextBox(parent, value), mId(id) {}

private:
    int mId;

};

class NANOGUI_EXPORT Console : public Widget {
public:
    enum class CAlignment {
        Left,
        Center,
        Right
    };

    Console(Widget *parent);

    bool editable() const { return mEditable; }
    void setEditable(bool editable);

    const std::string &value() const { return mValue; }
    void setValue(const std::string &value) { mValue = value; }

    const std::string &defaultValue() const { return mDefaultValue; }
    void setDefaultValue(const std::string &defaultValue) { mDefaultValue = defaultValue; }

    CAlignment alignment() const { return mAlignment; }
    void setAlignment(CAlignment align) { mAlignment = align; }

    /// Return the underlying regular expression specifying valid formats
    const std::string &format() const { return mFormat; }
    /// Specify a regular expression specifying valid formats
    void setFormat(const std::string &format) { mFormat = format; }

    /// Set the \ref Theme used to draw this widget
    virtual void setTheme(Theme *theme) override;

    /// Set the change callback
    std::function<bool(const std::string& str)> callback() const { return mCallback; }
    void setCallback(const std::function<bool(const std::string& str)> &callback) { mCallback = callback; }

    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) override;
    virtual bool mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers) override;
    virtual bool mouseDragEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers) override;
    virtual bool mouseEnterEvent(const Vector2i &p, bool enter) override;
    virtual bool focusEvent(bool focused) override;
    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) override;
    virtual bool keyboardCharacterEvent(unsigned int codepoint) override;

    virtual Vector2i preferredSize(NVGcontext *ctx) const override;
    virtual void draw(NVGcontext* ctx) override;
    virtual void save(Serializer &s) const override;
    virtual bool load(Serializer &s) override;
protected:
    bool checkFormat(const std::string& input,const std::string& format);
    bool copySelection();
    void pasteFromClipboard();
    bool deleteSelection();

    void updateCursor(NVGcontext *ctx, float lastx,
                      const NVGglyphPosition *glyphs, int size);
    float cursorIndex2Position(int index, float lastx,
                               const NVGglyphPosition *glyphs, int size);
    int position2CursorIndex(float posx, float lastx,
                             const NVGglyphPosition *glyphs, int size);
protected:
    bool mEditable;
    bool mCommitted;
    std::string mValue;
    std::string mDefaultValue;
    CAlignment mAlignment;
    std::string mFormat;
    std::function<bool(const std::string& str)> mCallback;
    bool mValidFormat;
    std::string mValueTemp;
    int mCursorPos;
    int mSelectionPos;
    Vector2i mMousePos;
    Vector2i mMouseDownPos;
    Vector2i mMouseDragPos;
    int mMouseDownModifier;
    float mTextOffset;
    double mLastClick;
    int mNumRows;
    std::vector<ConsoleRow *> mRows;
};

NAMESPACE_END(nanogui)
