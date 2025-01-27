
#include "pixelwindow/window.h"
#include "util.h"

class Application : public ap::PixelWindow
{
protected:
    void OnCreate(unsigned width, unsigned height) override
    {
        xCentre_ = width >> 1;
        yCentre_ = height >> 1;
    }

    bool OnUpdateFrame(double /*millis*/) override
    {
        constexpr unsigned size = 100;

        ClearCanvas(COLOR_GREY);
        DrawRect(xCentre_ - (size >> 1), yCentre_ - (size >> 1), size, size, COLOR_RED, true);
        return true;
    }

private:
    int xCentre_ = 0;
    int yCentre_ = 0;
};

int main()
{
    Application app;
    app.Show(600, 400, "PixelWindow");
    return 0;
}
