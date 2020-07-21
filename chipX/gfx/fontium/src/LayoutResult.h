#pragma once

struct LayoutChar {
    uint symbol;
    int x, y, w, h;
    LayoutChar(uint s,int x,int y,int w,int h) :
            symbol(s),x(x),y(y),w(w),h(h) { }
    LayoutChar(uint s,int w,int h) :
            symbol(s),x(0),y(0),w(w),h(h) { }
    LayoutChar() : symbol(0),x(0),y(0),w(0),h(0) {}
};

struct LayoutResult
{
    int width=0, height=0;
    std::vector<LayoutChar> placed{};
};
