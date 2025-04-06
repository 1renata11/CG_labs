#include <wx/wx.h>
#include <wx/graphics.h>
#include <vector>
#include <iostream>

void DrawPixel(int x, int y, wxColour color) {
    wxClientDC dc(wxTheApp->GetTopWindow());
    dc.SetPen(wxPen(color));
    dc.DrawPoint(x, y);
}

void DrawLine(int x1, int y1, int x2, int y2) {
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
                DrawPixel(x, y, *wxBLACK);
                if (e >= 0) {
                    y += iy;
                    e -= 2 * dx;
                }
                x += ix;
                e += dy * 2;
            }
        } else {
            for (i = 0; i <= dx; i++) {
                DrawPixel(x, y, *wxBLACK);
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
                DrawPixel(x, y, *wxBLACK);
                if (e >= 0) {
                    x += ix;
                    e -= 2 * dy;
                }
                y += iy;
                e += dx * 2;
            }
        } else {
            for (i = 0; i <= dy; i++) {
                DrawPixel(x, y, *wxBLACK);
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
        DrawLine(x1, y1, x2, y2);
    }
}

bool LinesCross(const wxPoint& p1, const wxPoint& p2, const wxPoint& q1, const wxPoint& q2) {
    auto cross = [](const wxPoint& p, const wxPoint& q) { return p.x * q.y - p.y * q.x; };
    wxPoint r = wxPoint(p2.x - p1.x, p2.y - p1.y);
    wxPoint s = wxPoint(q2.x - q1.x, q2.y - q1.y);
    int d = cross(r, s);
    if (d == 0) return false;
    wxPoint qp = wxPoint(q1.x - p1.x, q1.y - p1.y);
    double t = cross(qp, s) / static_cast<double>(d);
    double u = cross(qp, r) / static_cast<double>(d);
    return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
}

bool IsPolygonSimple(const std::vector<wxPoint>& points) {
    int n = points.size();
    for (int i = 0; i < n; ++i) {
        for (int j = i + 2; j < n; ++j) {
            if ((i == 0 && j == n - 1)) continue;
            if (LinesCross(points[i], points[(i + 1) % n], points[j], points[(j + 1) % n])) {
                return false;
            }
        }
    }
    return true;
}

bool IsPolygonConvex(const std::vector<wxPoint>& points) {
    auto cross = [](const wxPoint& p, const wxPoint& q, const wxPoint& q2) { return (q.x-p.x) * (q2.y-q.y) - (q.y - p.y)*(q2.x-q.x); };
    int n = points.size();
    int prev = 0;
    int curr = 0;
    for (int i = 0; i < n; i++) {
        std::vector<wxPoint > temp
                = { points[i],
                    points[(i + 1) % n],
                    points[(i + 2) % n] };
        int curr = cross(temp[0], temp[1], temp[2]);
        if (curr != 0) {
            if (curr * prev < 0) {
                return false;
            }
            else {
                prev = curr;
            }
        }
    }
    return true;
}

bool PointInPolygonEvenOdd(int x, int y, const std::vector<wxPoint>& points) {
    int n = points.size();
    int count = 0;
    for (int i = 0; i < n; i++) {
        wxPoint p1 = points[i];
        wxPoint p2 = points[(i + 1) % n];
        if ((y > std::min(p1.y, p2.y))
            && (y <= std::max(p1.y, p2.y))
            && (x <= std::max(p1.x, p2.x))) {
            double xIntersect = (y - p1.y)
                                * (p2.x - p1.x)
                                / (p2.y - p1.y)
                                + p1.x;
            if (p1.x == p2.x || x <= xIntersect) {
                count++;
            }
        }
    }
    return count % 2 == 1;
}

int crossProduct(const wxPoint& p1, const wxPoint& p2,
                    const wxPoint& p3)
{
    return (p2.x - p1.x) * (p3.y - p1.y)
           - (p2.y - p1.y) * (p3.x - p1.x);
}
bool isPointOnSegment(const wxPoint& p, const wxPoint& p1,
                      const wxPoint& p2)
{
    return crossProduct(p1, p2, p) == 0
           && p.x >= std::min(p1.x, p2.x)
           && p.x <= std::max(p1.x, p2.x)
           && p.y >= std::min(p1.y, p2.y)
           && p.y <= std::max(p1.y, p2.y);
}

bool PointInPolygonNonZero(int x, int y, const std::vector<wxPoint>& points) {
    int n = points.size();
    int windingNumber = 0;
    wxPoint point(x, y);
    for (int i = 0; i < n; i++) {
        wxPoint p1 = points[i];
        wxPoint p2 = points[(i + 1) % n];
        if (isPointOnSegment(point, p1, p2)) {
            return 0;
        }
        if (p1.y <= point.y) {
            if (p2.y > point.y
                && crossProduct(p1, p2, point) > 0) {
                windingNumber++;
            }
        }
        else {
            if (p2.y <= point.y
                && crossProduct(p1, p2, point) < 0) {
                windingNumber--;
            }
        }
    }
    return windingNumber!=0;
}

void FillPolygonEvenOdd(const std::vector<wxPoint>& points, wxColour color) {
    for (int y = 0; y < 600; y++) {
        for (int x = 0; x < 800; x++) {
            if (PointInPolygonEvenOdd(x, y, points)) {
                DrawPixel(x, y, color);
            }
        }
    }
}

void FillPolygonNonZero(const std::vector<wxPoint>& points, wxColour color) {
    for (int y = 0; y < 600; y++) {
        for (int x = 0; x < 800; x++) {
            if (PointInPolygonNonZero(x, y, points)) {
                DrawPixel(x, y, color);
            }
        }
    }
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
    DrawLine(700, 500, 790, 590);
    DrawLine(790, 590, 700, 500);
    std::vector<wxPoint> starPolygon1 = {
            {1, 1}, {80, 50}, {1, 50},
            {80, 1}, {40, 90}
    };
    DrawPolygon(starPolygon1);
    FillPolygonNonZero(starPolygon1, *wxBLACK);
    std::vector<wxPoint> starPolygon2 = {
            {101, 101}, {180, 150}, {101, 150},
            {180, 101}, {140, 190}
    };
    DrawPolygon(starPolygon2);
    FillPolygonEvenOdd(starPolygon2, *wxBLACK);
    std::vector<wxPoint> polygon2;
    polygon2.push_back(wxPoint(350, 400));
    polygon2.push_back(wxPoint(500, 380));
    polygon2.push_back(wxPoint(400, 400));
    polygon2.push_back(wxPoint(50, 550));
    polygon2.push_back(wxPoint(500, 300));
    if (IsPolygonSimple(polygon2)) std::cout << "Polygon2 is simple" << std::endl;
    else std::cout << "Polygon2 is not simple" << std::endl;
    if (IsPolygonConvex(polygon2)) std::cout << "Polygon2 is convex" << std::endl;
    else std::cout << "Polygon2 is not convex" << std::endl;
    DrawPolygon(polygon2);
    FillPolygonNonZero(polygon2, *wxGREEN);
    std::vector<wxPoint> polygon3;
    polygon3.push_back(wxPoint(550, 10));
    polygon3.push_back(wxPoint(530, 50));
    polygon3.push_back(wxPoint(600, 120));
    polygon3.push_back(wxPoint(750, 200));
    polygon3.push_back(wxPoint(720, 150));
    polygon3.push_back(wxPoint(780, 300));
    polygon3.push_back(wxPoint(420, 250));
    polygon3.push_back(wxPoint(580, 130));
    if (IsPolygonSimple(polygon3)) std::cout << "Polygon3 is simple" << std::endl;
    else std::cout << "Polygon3 is not simple" << std::endl;
    if (IsPolygonConvex(polygon3)) std::cout << "Polygon3 is convex" << std::endl;
    else std::cout << "Polygon3 is not convex" << std::endl;
    DrawPolygon(polygon3);
    FillPolygonNonZero(polygon3, *wxBLUE);
}
