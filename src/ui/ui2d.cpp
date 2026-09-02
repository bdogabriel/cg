#include "ui2d.h"

// TODO: think about a better desing

#include "font.h"
#include "text.h"
#include <cstdio>

namespace ui2d
{
static const float TEXT_SCALE = 0.75f;
static const int BAR_PADDING = 4;

constexpr Color status_bg = {41, 46, 61, 255};
constexpr Color prompt_bg = {15, 15, 20, 242};
constexpr Color border_color = {128, 128, 153, 204};
constexpr Color text_color = {245, 245, 255, 255};
constexpr Color accent_color = {77, 204, 230, 255};

static void draw_bar(QuadBatch &batch, const Font &font, float x, float y, float w, float h, const Color &bg)
{
    text::fill_rect(batch, font, x, y, w, h, bg);
    text::outline_rect(batch, font, x, y, w, h, border_color);
}

static void draw_text_line(QuadBatch &batch, const Font &font, float x, float y, const char *text, const Color &color)
{
    text::append_text(batch, font, text, x, y, color, TEXT_SCALE);
}

static void append_upper(char *dst, size_t cap, const char *src)
{
    size_t len = strlen(dst);
    while (len + 1 < cap && *src)
    {
        char c = *src;
        if (c >= 'a' && c <= 'z')
        {
            c -= ('a' - 'A');
        }
        dst[len++] = c;
        src++;
    }
    dst[len] = '\0';
}

static const char *target_name(Target t)
{
    return (t == Target::Face) ? "face" : "object";
}

void build_statusline(const Editor &e, const Session &s, int fbW, int fbH, QuadBatch &batch)
{
    const Font &font = baked_font();
    float lineHeight = font.lineHeight * TEXT_SCALE;
    float barHeight = lineHeight + BAR_PADDING;
    float statusY = fbH - barHeight;
    draw_bar(batch, font, 0.0f, statusY, fbW, barHeight, status_bg);
    char status[256];
    status[0] = '\0';
    const char *cmdName = s.cmd ? s.cmd->name : "none";
    const char *axis = s.args.get("axis");
    if (strcmp(cmdName, "none") != 0)
    {
        append_upper(status, sizeof(status), cmdName);
        append_upper(status, sizeof(status), " ");
    }
    append_upper(status, sizeof(status), target_name(e.target));
    if (axis && axis[0] != '\0')
    {
        append_upper(status, sizeof(status), " ");
        append_upper(status, sizeof(status), axis);
    }
    if (e.locked)
    {
        append_upper(status, sizeof(status), " ");
        append_upper(status, sizeof(status), "LOCKED");
    }
    draw_text_line(batch, font, 8.0f, statusY + BAR_PADDING / 2.0f, status, text_color);
}

void build_prompt(const Session &s, int fbW, int fbH, QuadBatch &batch)
{
    const Font &font = baked_font();
    float lineHeight = font.lineHeight * TEXT_SCALE;
    float barHeight = lineHeight + BAR_PADDING;
    float statusY = fbH - barHeight;
    float promptY = statusY - barHeight;
    draw_bar(batch, font, 0.0f, promptY, fbW, barHeight, prompt_bg);
    char promptText[1024];
    snprintf(promptText, sizeof(promptText), ":%s", s.prompt.buffer);
    draw_text_line(batch, font, 8.0f, promptY + BAR_PADDING / 2.0f, promptText, accent_color);
}

} // namespace ui2d
