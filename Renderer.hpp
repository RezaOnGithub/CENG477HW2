// The Canvas Interface MUST Have:
//    - putpixel(i,j) that silently NOPs if out of bounds
//    - getw() geth() returning width and height

template<class Canvas>
class Renderer
{
public:
    Renderer(Canvas& canvas, bool bfc) :
        c(canvas),
        backface_culling(bfc)
    {
    }

    void go()
    {
    }
private:
    Canvas& c;
    bool backface_culling;
};
