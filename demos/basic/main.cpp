
#include "pixelwindow/window.h"

class Application : public ap::PixelWindow
{
protected:
    bool OnCreate(unsigned width, unsigned height) override
    {
        xCentre_ = width / 2;
        yCentre_ = height / 2;
        return true;
    }

    bool OnUpdateFrame([[maybe_unused]] double millis) override
    {
        constexpr unsigned size = 100;

        ClearCanvas(COLOR_GREY);
        DrawRect(xCentre_ - (size / 2), yCentre_ - (size / 2), size, size, COLOR_RED, true);

        // Canvas was updated
        return true;
    }

private:
    unsigned xCentre_ = 0;
    unsigned yCentre_ = 0;
};

int main()
{
    Application app;

    if (!app.Show(600, 400, "Basic")) {
        return 1;
    }

    return 0;
}
