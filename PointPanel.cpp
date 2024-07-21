#include "PointPanel.h"
#include <wx/dcclient.h>
#include<wx/dcbuffer.h> 
#include <wx/math.h>

wxBEGIN_EVENT_TABLE(PointPanel, wxPanel)
    EVT_PAINT(PointPanel::OnPaint)
    EVT_MOUSEWHEEL(PointPanel::OnMouseWheel)
    EVT_LEFT_DOWN(PointPanel::OnMouseLeftDown)
    EVT_LEFT_UP(PointPanel::OnMouseLeftUp)
    EVT_MOTION(PointPanel::OnMouseDrag)
wxEND_EVENT_TABLE()

PointPanel::PointPanel(wxWindow* parent)
    : wxPanel(parent)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT); // Potrebno za double buffering: lepse slikanje tock
}

void PointPanel::SetPoints(const std::vector<wxPoint>& points)
{
    points_ = points;
    hull_.clear();        // ob ponovnem zagonu tock, se pretekli hull izbrise
    pointsDirty_ = true;  // za to, da triggeram bitmap update
    Refresh();
}

void PointPanel::SetHull(const std::vector<wxPoint>& hull)
{
    hull_ = hull;
    Refresh();
}

void PointPanel::SetZoom(double zoom)
{
    zoom_ = zoom;
    Refresh();
}

void PointPanel::SetOffset(const wxPoint& offset)
{
    offset_ = offset;
    Refresh();
}

void PointPanel::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);  // double buffering
    dc.Clear();

    wxSize panelSize = GetClientSize();

    // Draw coordinate system
    dc.SetPen(*wxLIGHT_GREY_PEN);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);

    // Narise tocke
    if (pointsDirty_)
    {
        pointsBitmap_ = wxBitmap(panelSize);
        wxMemoryDC memDC(pointsBitmap_);
        memDC.Clear();

        memDC.SetBrush(*wxBLACK_BRUSH);
        memDC.SetPen(*wxBLACK_PEN);

        for (const wxPoint& point : points_)
        {
            wxPoint adjustedPoint = wxPoint(
                panelSize.GetWidth() / 2 + (point.x * zoom_) - offset_.x,
                panelSize.GetHeight() / 2 - (point.y * zoom_) - offset_.y
            );

            memDC.DrawCircle(adjustedPoint, 1);
        }

        pointsDirty_ = false;
    }

    // Narise tocke bitmap
    dc.DrawBitmap(pointsBitmap_, 0, 0);

    // Draw convex hull
    if (!hull_.empty())
    {
        dc.SetPen(wxPen(*wxRED, 1));
        for (size_t i = 0; i < hull_.size(); ++i)
        {
            wxPoint p1 = wxPoint(
                panelSize.GetWidth() / 2 + (hull_[i].x * zoom_) - offset_.x,
                panelSize.GetHeight() / 2 - (hull_[i].y * zoom_) - offset_.y
            );

            wxPoint p2 = wxPoint(
                panelSize.GetWidth() / 2 + (hull_[(i + 1) % hull_.size()].x * zoom_) - offset_.x,
                panelSize.GetHeight() / 2 - (hull_[(i + 1) % hull_.size()].y * zoom_) - offset_.y
            );

            dc.DrawLine(p1, p2);
        }
    }
}

// ---------------------------------------------------------------
// Spodnje kode opisujejo premikanje koordinatnega sistema z misko
// ---------------------------------------------------------------

void PointPanel::OnMouseWheel(wxMouseEvent& event)
{
    double zoomChange = (event.GetWheelRotation() > 0) ? 1.1 : 0.9;
    SetZoom(zoom_ * zoomChange);
}

void PointPanel::OnMouseLeftDown(wxMouseEvent& event)
{
    dragStart_ = event.GetPosition();
    CaptureMouse();
}

void PointPanel::OnMouseLeftUp(wxMouseEvent& event)
{
    if (HasCapture()) {
        ReleaseMouse();
    }
}

void PointPanel::OnMouseDrag(wxMouseEvent& event)
{
    if (HasCapture() && event.Dragging()) 
    {
        wxPoint dragEnd = event.GetPosition();
        wxPoint delta = dragEnd - dragStart_;

        SetOffset(wxPoint(offset_.x + delta.x, offset_.y + delta.y));
        dragStart_ = dragEnd;
    }
}
