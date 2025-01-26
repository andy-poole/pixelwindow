
#include "pixelwindow/window.h"
#include "util.h"

class Application : public ap::PixelWindow
{
protected:
    void OnCreate(unsigned /*width*/, unsigned /*height*/) override
    {
    }

    bool OnUpdateFrame(double /*millis*/) override
    {
        ClearCanvas(COLOR_GREY);
        return true;
    }
};

int main()
{
    Application app;
    app.Show(600, 400, "PixelWindow");
    return 0;
}
