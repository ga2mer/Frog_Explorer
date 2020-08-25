#include <borealis.hpp>

class StatusCell : public brls::View {
 private:
  std::string label;
  std::string value;

  NVGcolor valueColor;

 public:
  StatusCell(std::string label, std::string value);

  void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height,
            brls::Style *style, brls::FrameContext *ctx) override;

  void setValue(std::string value);
  void setValueColor(NVGcolor color);
  void resetValueColor();
};