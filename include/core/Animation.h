#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <raylib.h>
#include <utils/Vector.h>

struct SpriteSheet {
    Texture2D texture;
    int       count;         // total de frames
    int       rowsCount;     // filas lógicas
    int       columnsCount;  // columnas lógicas
    int       frameW;        // ancho de un frame (px)
    int       frameH;        // alto  de un frame (px)
    int       sepX;          // separación horizontal entre frames
    int       sepY;          // separación vertical  entre frames
    Rectangle src;           // rect de inicio en el atlas
    bool      isVertical;    // true → frames leídos en columna
    bool      reverse;       // true → animación al revés
};

class Animation {
public:
    int Index = 0;

    Animation(const SpriteSheet& sheet, float msPerFrame, bool loop)
        : sheet_(sheet), msPerFrame_(msPerFrame), loop_(loop),
          elapsed_(0.0f), playing_(false)
    {
        buildFrames();
    }

    void Play()  { elapsed_ = 0.0f; Index = 0; playing_ = true; }
    void Stop()  { playing_ = false; }
    bool IsPlaying() const { return playing_; }

    void Update(float dt) {
        if (!playing_) return;
        if (!loop_ && Index == (int)srcs_.size() - 1) {
            playing_ = false;
            return;
        }
        elapsed_ += dt * 1000.0f;
        if (elapsed_ >= msPerFrame_) {
            elapsed_ = 0.0f;
            Index = (Index + 1) % (int)srcs_.size();
        }
    }

    void DrawAt(Rectangle dst, Color tint = WHITE) const {
        if (srcs_.empty()) return;
        DrawTexturePro(sheet_.texture, srcs_[Index], dst, {0,0}, 0.0f, tint);
    }

    void DrawCenteredAtWithScale(float x, float y, float s) const {
        if (srcs_.empty()) return;
        float w = sheet_.frameW * s;
        float h = sheet_.frameH * s;
        DrawTexturePro(
            sheet_.texture,
            srcs_[Index],
            Rectangle{ x - w/2.0f, y - h/2.0f, w, h },
            {0, 0}, 0.0f, WHITE
        );
    }

private:
    void buildFrames() {
        srcs_.resize(sheet_.count);
        for (int i = 0; i < sheet_.count; i++) {
            Rectangle r = { 0, 0, (float)sheet_.frameW, (float)sheet_.frameH };

            if (!sheet_.isVertical) {
                r.x = sheet_.src.x + (float)(i % sheet_.rowsCount)    * sheet_.frameW
                                   + (float)(i * sheet_.sepX);
                r.y = sheet_.src.y + (float)(i / sheet_.rowsCount)    * sheet_.frameH
                                   + (float)(i * sheet_.sepY);
            } else {
                r.x = sheet_.src.x + (float)(i / sheet_.columnsCount) * sheet_.frameW
                                   + (float)(i * sheet_.sepX);
                r.y = sheet_.src.y + (float)(i % sheet_.columnsCount) * sheet_.frameH
                                   + (float)(i * sheet_.sepY);
            }

            int dst_i = sheet_.reverse ? (sheet_.count - 1 - i) : i;
            srcs_[dst_i] = r;
        }
    }

    SpriteSheet            sheet_;
    Vector<Rectangle> srcs_;
    float                  msPerFrame_;
    bool                   loop_;
    float                  elapsed_;
    bool                   playing_;
};

inline SpriteSheet NewSpriteSheet(
    Texture2D tex,
    int count, int rows, int cols,
    int fw, int fh,
    int sepX, int sepY,
    Rectangle src,
    bool isVertical, bool reverse)
{
    return SpriteSheet{ tex, count, rows, cols, fw, fh, sepX, sepY, src, isVertical, reverse };
}

inline Animation* NewAnimation(const SpriteSheet& sh, float ms, bool loop) {
    return new Animation(sh, ms, loop);
}

#endif