// Объявите этот макрос в самом начале файла, чтобы при подключении <cmath> были объявлены макросы M_PI и другие 
#define _USE_MATH_DEFINES 
#include <cmath>
#include "svg.h"

namespace svg {

using namespace std::literals;

//Rgb

Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b) 
: red(r)
, green(g)
, blue(b) {
}

//Rgba

Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double op) 
: Rgb(r, g, b)
, opacity(op) {
}

//ColorVisitor

void ColorVisitor::operator()(std::monostate) {
    os << "none"sv;
}
void ColorVisitor::operator()(std::string color) {
    os << color;
}
void ColorVisitor::operator()(Rgb color) {
    os << "rgb("sv << static_cast<int>(color.red) << ","sv << static_cast<int>(color.green) << ","sv << static_cast<int>(color.blue) << ")"sv;
}
void ColorVisitor::operator()(Rgba color) {
    os << "rgba("sv << static_cast<int>(color.red) << ","sv << static_cast<int>(color.green) << ","sv << static_cast<int>(color.blue) << ","sv << color.opacity << ")"sv;
}


//ostream Color
std::ostream& operator<<(std::ostream& os, const Color& color) {
    std::visit(ColorVisitor{os}, color);
    return os;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ----------Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for(auto& obj : objects) {
        obj.get()->Render(out);
    }
    out << "</svg>"sv;
}

// ----------Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "  <circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ----------Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    peaks_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "  <polyline points=\""sv;
    size_t curr_pos = 0;
    for(const Point& peak : peaks_) {
        if(curr_pos == peaks_.size() - 1) {
            break;
        }
        out << peak.x << ","sv << peak.y << " "sv;
        ++curr_pos;
    }
    if(curr_pos != peaks_.size()) {
        out << peaks_.back().x << ","sv << peaks_.back().y << "\""sv; 
        RenderAttrs(out);
        out << "/>"sv;
        return;
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ------------Text ------------------

Text& Text::SetPosition(Point pos) {
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "  <text"sv;
    RenderAttrs(out);
    out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv << font_size_ << "\""sv;
    if (font_family_ != "" || font_weight_ != "") {
        if (font_family_ != "") {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (font_weight_ != "") {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
    }
    out << ">"sv;
    for (char c : data_) {
        switch (c) {
            case '"': out << "&quot;"; break;
            case '\'': out << "&apos;"; break;
            case '<': out << "&lt;"; break;
            case '>': out << "&gt;"; break;
            case '&': out << "&amp;"; break;
            default: out << c;
        }
    }
    out << "</text>"sv;
}

//Реализация функции CreateStar 
Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) {
    Polyline polyline;
    for(int i = 0; i <= num_rays; ++i) {
        double angle = 2 * M_PI * (i % num_rays) / num_rays;
        polyline.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)});
        if(i == num_rays) {
            break;
        }
        angle += M_PI / num_rays;
        polyline.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)});
    }
    return polyline;
}

std::ostream& operator<<(std::ostream& os, const StrokeLineCap& line_cap) {
    switch(line_cap) {
        case StrokeLineCap::BUTT:
            os << "butt";
            break;
        case StrokeLineCap::ROUND:
            os << "round";
            break;
        case StrokeLineCap::SQUARE:
            os << "square";
            break;
        default:
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& line_join) {
    switch(line_join) {
        case svg::StrokeLineJoin::ARCS:
            os << "arcs";
            break;
        case svg::StrokeLineJoin::BEVEL:
            os << "bevel";
            break;
        case svg::StrokeLineJoin::MITER:
            os << "miter";
            break;
        case svg::StrokeLineJoin::MITER_CLIP:
            os << "miter-clip";
            break;
        case svg::StrokeLineJoin::ROUND:
            os << "round";
            break;
        default:
            break;
    }
    return os;
}

}  // namespace svg

namespace shapes{

    //Реализация методов и конструктора класса Triangle
    Triangle::Triangle(svg::Point p1, svg::Point p2, svg::Point p3) 
    : p1_(p1)
    , p2_(p2)
    , p3_(p3){
    }

    void Triangle::Draw(svg::ObjectContainer& container) const {
        container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
    }

    //Реализация методов и конструктора класса Star
    Star::Star(svg::Point center, double outer_radius, double inner_radius, int num_rays) 
    : center_(center)
    , outer_radius_(outer_radius)
    , inner_radius_(inner_radius)
    , num_rays_(num_rays){
    }
    
    void Star::Draw(svg::ObjectContainer& container) const {
        container.Add(svg::CreateStar(center_, outer_radius_, inner_radius_, num_rays_).SetFillColor("red").SetStrokeColor("black"));
    }

    //Реализация методов и конструктора Snowman
    Snowman::Snowman(svg::Point head_center, double head_radius) 
    : head_center_(head_center)
    , head_radius_(head_radius){
    }

    void Snowman::Draw(svg::ObjectContainer& container) const {
        container.Add(svg::Circle().SetCenter({head_center_.x, head_center_.y + 5 * head_radius_}).SetRadius(head_radius_ * 2).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
        container.Add(svg::Circle().SetCenter({head_center_.x, head_center_.y + 2 * head_radius_}).SetRadius(head_radius_ * 1.5).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
        container.Add(svg::Circle().SetCenter(head_center_).SetRadius(head_radius_).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
    }

} // namespace shapes