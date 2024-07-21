#pragma once
#include <wx/wx.h>
#include <vector>

class PointPanel : public wxPanel {
public:
    PointPanel(wxWindow* parent);

    void SetPoints(const std::vector<wxPoint>& points);
    void SetHull(const std::vector<wxPoint>& hull);

    void SetZoom(double zoom);
    void SetOffset(const wxPoint& offset);

private:
    void OnPaint(wxPaintEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnMouseLeftDown(wxMouseEvent& event);
    void OnMouseLeftUp(wxMouseEvent& event);
    void OnMouseDrag(wxMouseEvent& event);

    std::vector<wxPoint> points_;
    std::vector<wxPoint> hull_;

    double zoom_ = 1.0;
    wxPoint offset_{ 0, 0 };
    wxPoint dragStart_;
    wxBitmap pointsBitmap_;
    wxMemoryDC pointsDC_;
    bool pointsDirty_ = true;

    wxDECLARE_EVENT_TABLE();
};
