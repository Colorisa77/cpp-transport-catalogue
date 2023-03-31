#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <deque>
#include <optional>
#include <variant>

namespace svg {

//Rgb
struct Rgb {
    Rgb() = default;
    Rgb(uint8_t r, uint8_t g, uint8_t b);

    ~Rgb() = default;
    
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

//Rgba
struct Rgba final : public Rgb {
    Rgba() = default;
    Rgba(uint8_t r, uint8_t g, uint8_t b, double op);

    double opacity = 1.0;
};

//Объявляем Color типа std::variant
using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    
inline const Color NoneColor{"none"};

std::ostream& operator<<(std::ostream& os, const Color& color);

struct ColorVisitor {
    std::ostream& os;

    void operator()(std::monostate);
    void operator()(std::string color);
    void operator()(Rgb color);
    void operator()(Rgba color);
};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& os, const StrokeLineCap& line_cap);
std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& line_join);

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};


//Интерфейсы Drawable и ObjectContainer
class ObjectContainer {
public:
    template <typename Obj>
    void Add(Obj obj) {
        objects.emplace_back(std::make_unique<Obj>(std::move(obj)));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

protected:
    std::deque<std::unique_ptr<Object>> objects;
    ~ObjectContainer() = default;
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};

//Добавим шаблонный класс PathProps для того, чтобы Circle, Text и Polyline унаследовали от него методы.

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        color_fill_ = color;
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = color;
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width) {
        width_ = width;
        return AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        line_cap_ = line_cap;
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = line_join;
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        if(color_fill_) {
            out << " fill=\"" << (*color_fill_) << "\"";
        }
        if(stroke_color_) {
            out << " stroke=\"" << (*stroke_color_) << "\"";
        }
        if(width_) {
            out << " stroke-width=\"" << (*width_) << "\"";
        }
        if(line_cap_) {
            out << " stroke-linecap=\"" << (*line_cap_) << "\"";
        }
        if(line_join_) {
            out << " stroke-linejoin=\"" << (*line_join_) << "\"";
        }
    }
private:
    Owner& AsOwner() {
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> color_fill_;
    std::optional<Color> stroke_color_;
    std::optional<double> width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
};


/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle() = default;
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_ = {0.0, 0.0};
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

    /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>
     */
private:

    void RenderObject(const RenderContext& context) const override;

    std::deque<Point> peaks_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:

    // Создаём конструктор, который по умолчанию инициализирует поля класса
    Text() = default;

    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    // Прочие данные и методы, необходимые для реализации элемента <text>
private:

    void RenderObject(const RenderContext& context) const override;

    Point position_ = {0.0, 0.0};
    Point offset_= {0.0, 0.0};
    uint32_t font_size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_ = "";
};

class Document final : public ObjectContainer {
public:

    Document() = default;
    /*
     Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
     Пример использования:
     Document doc;
     doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
    */

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

    // Прочие методы и данные, необходимые для реализации класса Document
};

//Функция CreateStar
Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays);

} // namespace svg

namespace shapes {

    //Класс Star
    class Star final : public svg::Drawable {
    public:
        Star(svg::Point center, double outer_radius, double inner_radius, int num_rays);
        void Draw(svg::ObjectContainer& container) const override;
    private:
        svg::Point center_;
        double outer_radius_;
        double inner_radius_;
        int num_rays_;
    };

    //Класс Snowman
    class Snowman final : public svg::Drawable {
    public:
        Snowman(svg::Point head_center, double head_radius);
        void Draw(svg::ObjectContainer& container) const override;
    private:
        svg::Point head_center_;
        double head_radius_;
    };

    //Класс Triangle
    class Triangle final : public svg::Drawable {
    public:
        Triangle(svg::Point p1, svg::Point p2, svg::Point p3);
        void Draw(svg::ObjectContainer& container) const override;
    private:
        svg::Point p1_, p2_, p3_;
    };
}