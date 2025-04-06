#include <wx/wx.h>
#include <wx/graphics.h>
#include <vector>
#include <iostream>
#include <wx/gdicmn.h>
#include <string>

void DrawPixel(int x, int y, wxColour color) {
    wxClientDC dc(wxTheApp->GetTopWindow());
    dc.SetPen(wxPen(color));
    dc.DrawPoint(x, y);
}

void DrawLine(int x1, int y1, int x2, int y2, wxColour color) {
    int x = x1, y = y1;
    int dx = x2 - x1, dy = y2 - y1;
    int ix, iy;
    int e;
    int i;
    if (dx > 0) ix = 1;
    else if (dx < 0) {
        ix = -1;
        dx = -dx;
    }
    else ix = 0;
    if (dy > 0) iy = 1;
    else if (dy < 0) {
        iy = -1;
        dy = -dy;
    }
    else iy = 0;
    if (dx >= dy) {
        e = 2 * dy - dx;
        if (iy >= 0) {
            for (i = 0; i <= dx; i++) {
                DrawPixel(x, y, color);
                if (e >= 0) {
                    y += iy;
                    e -= 2 * dx;
                }
                x += ix;
                e += dy * 2;
            }
        } else {
            for (i = 0; i <= dx; i++) {
                DrawPixel(x, y, color);
                if (e > 0) {
                    y += iy;
                    e -= 2 * dx;
                }
                x += ix;
                e += dy * 2;
            }
        }
    } else
    {
        e = 2 * dx - dy;
        if (ix >= 0) {
            for (i = 0; i <= dy; i++) {
                DrawPixel(x, y, color);
                if (e >= 0) {
                    x += ix;
                    e -= 2 * dy;
                }
                y += iy;
                e += dx * 2;
            }
        } else {
            for (i = 0; i <= dy; i++) {
                DrawPixel(x, y, color);
                if (e > 0) {
                    x += ix;
                    e -= 2 * dy;
                }
                y += iy;
                e += dx * 2;
            }
        }
    }
}

void DrawPolygon(const std::vector<wxPoint>& points) {
    int n = points.size();
    for (int i = 0; i < n; i++) {
        int x1 = points[i].x;
        int y1 = points[i].y;
        int x2 = points[(i + 1) % n].x;
        int y2 = points[(i + 1) % n].y;
        DrawLine(x1, y1, x2, y2, *wxBLACK);
    }
}

void DrawBezier(const wxPoint& p0, const wxPoint& p1, const wxPoint& p2, const wxPoint& p3, wxColour color) {
    double maxDist = std::max({hypot(p1.x - p0.x, p1.y - p0.y),
                               hypot(p2.x - p1.x, p2.y - p1.y),
                               hypot(p3.x - p2.x, p3.y - p2.y)});
    double step = 1.0 / maxDist; 

    for (double t = 0; t <= 1; t += step) {
        double x = (1 - t) * (1 - t) * (1 - t) * p0.x +
                   3 * (1 - t) * (1 - t) * t * p1.x +
                   3 * (1 - t) * t * t * p2.x +
                   t * t * t * p3.x;
        double y = (1 - t) * (1 - t) * (1 - t) * p0.y +
                   3 * (1 - t) * (1 - t) * t * p1.y +
                   3 * (1 - t) * t * t * p2.y +
                   t * t * t * p3.y;
        DrawPixel(static_cast<int>(x), static_cast<int>(y), color);
    }

    DrawLine(p0.x, p0.y, p1.x, p1.y, color);
    DrawLine(p1.x, p1.y, p2.x, p2.y, color);
    DrawLine(p2.x, p2.y, p3.x, p3.y, color);
}

bool IsPolygonCW(const std::vector<wxPoint>& polygon) {
    double x1=polygon[1].x-polygon[0].x;
    double y1=polygon[1].y-polygon[0].y;
    double x2=polygon[2].x-polygon[1].x;
    double y2=polygon[2].y-polygon[1].y;
    double vec_prod=x1*y2-y1*x2;
    if(vec_prod>0) return true;
    return false;
}

bool CyrusBeckClipLine(const wxPoint& p1, const wxPoint& p2, const std::vector<wxPoint>& polygon, wxPoint& clippedStart, wxPoint& clippedEnd) {
    bool CW=IsPolygonCW(polygon);
    if (CW) {std::cout << "CW" << std::endl;}
    else {std::cout << "Not CW" << std::endl;}
    double t1 = 0.0, t2 = 1.0, t;
    double sx = p2.x - p1.x, sy = p2.y - p1.y;
    double nx, ny, denom, num;

    for (size_t i = 0; i < polygon.size(); i++) {
        nx = -polygon[(i + 1) % polygon.size()].y + polygon[i].y;
        ny = -polygon[i].x + polygon[(i + 1) % polygon.size()].x;
        if (!CW) {nx=-nx; ny=-ny;}

        denom = nx * sx + ny * sy;
        num = nx * (p1.x - polygon[i].x) + ny * (p1.y - polygon[i].y);

        if (denom != 0) {
            t = -num / denom;
            if (denom > 0) {
                if (t >= 0.0 && t <= 1.0) {
                    t1 = std::max(t1, t);
                }
            } else {
                if (t >= 0.0 && t <= 1.0) {
                    t2 = std::min(t2, t);
                }
            }
        }
        else {
            if (num<0) {return false; }
        }
    }

    if (t1 <= t2) {
        clippedStart = wxPoint(p1.x + t1 * (p2.x - p1.x), p1.y + t1 * (p2.y - p1.y));
        return true;
    }
    return false;
}


class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);

private:
    void OnPaint(wxPaintEvent& evt);
wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
                EVT_PAINT(MyFrame::OnPaint)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    MyFrame* frame = new MyFrame("Drawing Polygons and Filling");
    frame->Show(true);
    return true;
}

MyFrame::MyFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)) {}

void MyFrame::OnPaint(wxPaintEvent& evt) {
    wxPaintDC dc(this);

    DrawBezier(wxPoint(100, 500), wxPoint(200, 100), wxPoint(400, 100), wxPoint(500, 500), *wxGREEN);
    //Точки находятся на прямой
    DrawBezier(wxPoint(100, 100), wxPoint(200, 200), wxPoint(300, 300), wxPoint(400, 400), *wxBLUE);
    //Петля, крайние точки совпадают
    DrawBezier(wxPoint(550, 550), wxPoint(200, 150), wxPoint(450, 200), wxPoint(550, 550), *wxRED);
    std::vector<wxPoint> polygon = {
            {100, 100}, {500, 100},
            {500, 400}, {100, 400}, {50, 200}
    };
    std::reverse(polygon.begin(), polygon.end());

    DrawPolygon(polygon);
    wxPoint clippedStart, clippedEnd;
    //Отрезок пересекает полигон насквозь
    std::cout << CyrusBeckClipLine(wxPoint(600, 50), wxPoint(400, 450), polygon, clippedStart, clippedEnd);
    DrawLine(clippedStart.x, clippedStart.y, clippedEnd.x, clippedEnd.y, *wxBLACK);
    //Отрезок полностью внутри и параллелен стороне
    std::cout << CyrusBeckClipLine(wxPoint(400, 101), wxPoint(400, 350), polygon, clippedStart, clippedEnd);
    DrawLine(clippedStart.x, clippedStart.y, clippedEnd.x, clippedEnd.y, *wxGREEN);
    //Отрезок входит снаружи и заканчивается внутри и параллелен стороне
    std::cout << CyrusBeckClipLine(wxPoint(300, 40), wxPoint(300, 450), polygon, clippedStart, clippedEnd);
    DrawLine(clippedStart.x, clippedStart.y, clippedEnd.x, clippedEnd.y, *wxRED);
    //Отрезок пересекает полигон насквозь и параллелен стороне
    std::cout << CyrusBeckClipLine(wxPoint(50, 200), wxPoint(550, 200), polygon, clippedStart, clippedEnd);
    DrawLine(clippedStart.x, clippedStart.y, clippedEnd.x, clippedEnd.y, *wxBLUE);
    //Отрезок полностью снаружи
    std::cout << CyrusBeckClipLine(wxPoint(10, 40), wxPoint(20, 50), polygon, clippedStart, clippedEnd);
    DrawLine(clippedStart.x, clippedStart.y, clippedEnd.x, clippedEnd.y, *wxYELLOW);
    //Отрезок частично совпадает со стороной
    std::cout << CyrusBeckClipLine(wxPoint(50, 100), wxPoint(300, 100), polygon, clippedStart, clippedEnd);
    DrawLine(clippedStart.x, clippedStart.y, clippedEnd.x, clippedEnd.y, *wxYELLOW);
    //Выходит из вершины (500, 400) и пересекает полигон
    std::cout << CyrusBeckClipLine(wxPoint(625, 500), wxPoint(50, 50), polygon, clippedStart, clippedEnd);
    DrawLine(clippedStart.x, clippedStart.y, clippedEnd.x, clippedEnd.y, *wxBLUE);
}
