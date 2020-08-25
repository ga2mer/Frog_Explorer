#include "StatusCell.hpp"

StatusCell::StatusCell(std::string label, std::string value)
    : label(label), value(value) {
  this->resetValueColor();
}

void StatusCell::setValue(std::string value) { this->value = value; }

void StatusCell::draw(NVGcontext *vg, int x, int y, unsigned width,
                      unsigned height, brls::Style *style,
                      brls::FrameContext *ctx) {
  unsigned padding = 5;

  // Label
  nvgBeginPath(vg);
  nvgFillColor(vg, a(ctx->theme->tableBodyTextColor));
  nvgFontSize(vg, 16);
  nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_BASELINE);
  nvgText(vg, x + padding, y + height / 2, this->label.c_str(), nullptr);

  // Value
  nvgBeginPath(vg);
  nvgFillColor(vg, a(this->valueColor));
  nvgFontSize(vg, 20);
  nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_BASELINE);
  nvgText(vg, x + width - padding, y + height / 2, this->value.c_str(),
          nullptr);
}

void StatusCell::setValueColor(NVGcolor color) { this->valueColor = color; }

void StatusCell::resetValueColor() {
  this->valueColor = brls::Application::getThemeValues()->textColor;
}