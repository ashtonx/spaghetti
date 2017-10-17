#include "nodes/node.h"

#include <cmath>
#include <iostream>

#include <QDebug>
#include <QGraphicsDropShadowEffect>

#include "ui/colors.h"
#include "ui/package_view.h"

namespace nodes {

constexpr qreal ICON_OFFSET{ 25.0 };
constexpr QSizeF ICON_SIZE{ 50.0 + ICON_OFFSET, 25.0 + ICON_OFFSET };
constexpr qreal ROUND_FACTOR{ 15.0 };

Node::Node(QGraphicsItem *a_parent)
  : QGraphicsItem{ a_parent }
{
  setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);
  //  m_boundingRect = QRectF(QPointF{ 0.0, 0.0 }, ICON_SIZE);

  QGraphicsDropShadowEffect *const effect{ new QGraphicsDropShadowEffect };
  //  effect->setColor(QColor(103, 117, 126, 255));
  QColor color(58, 66, 71, 255); // DARK GREY
  effect->setColor(color);
  //  effect->setOffset(0.0, 0.0);
  //  effect->setBlurRadius(64.0);
  effect->setBlurRadius(15.0);
  effect->setColor(QColor("#99121212"));
  setGraphicsEffect(effect);

  iconify();
}

Node::~Node() {}

QRectF Node::boundingRect() const
{
  return m_boundingRect;
}

void Node::paint(QPainter *a_painter, QStyleOptionGraphicsItem const *a_option, QWidget *a_widget)
{
  (void)a_painter;
  (void)a_option;
  (void)a_widget;

  //  QPen pen{ color };
  QPen pen{ get_color(Color::eSocketBorder) };
  pen.setWidth(2);
  QColor color{ 105, 105, 105, 128 };
  QBrush brush{ color };
  a_painter->setPen(pen);
  a_painter->setBrush(brush);
  a_painter->drawRoundedRect(m_boundingRect, ROUND_FACTOR, ROUND_FACTOR);

  //  pen.setColor(QColor(54, 54, 54, 255));
  //  pen.setColor(isSelected() ? QColor(95, 124, 136, 255) : QColor(58, 66, 71, 255));
  pen.setColor(isSelected() ? QColor(156, 156, 156, 255) : QColor(58, 66, 71, 255));
  pen.setWidth(2);
  a_painter->setPen(pen);
  a_painter->setBrush(Qt::NoBrush);
  a_painter->drawRoundedRect(m_boundingRect, ROUND_FACTOR, ROUND_FACTOR);

  auto size = m_icon.size();
  auto size2 = size / 2.0;

  //  a_painter->translate(m_boundingRect.width() / 2.0 - size.width() / 4.0, m_boundingRect.height() / 2.0 -
  //  size.height() / 4.0); a_painter->drawPixmap(0, 0, size.width() / 2.0, size.height() / 2.0, m_icon);
  auto const x = static_cast<int>(-size2.width() / 2.0);
  auto const y = static_cast<int>(-size2.height() / 2.0);
  auto const w = size2.width();
  auto const h = size2.height();
  a_painter->drawPixmap(x, y, w, h, m_icon);
}

QVariant Node::itemChange(QGraphicsItem::GraphicsItemChange a_change, QVariant const &a_value)
{
  (void)a_change;
  (void)a_value;

  switch (a_change) {
    case QGraphicsItem::ItemSelectedChange: {
      break;
    }
    case QGraphicsItem::ItemPositionChange: {
      if (m_packageView && m_packageView->snapToGrid()) {
        QPointF const position{ a_value.toPointF() };
        qreal const x{ std::round(position.x() / 10.0) * 10.0 };
        qreal const y{ std::round(position.y() / 10.0) * 10.0 };
        return QPointF{ x, y };
      }
    }
    default: break;
  }

  return QGraphicsItem::itemChange(a_change, a_value);
}

void Node::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *a_event)
{
  (m_mode == Mode::eIconified) ? expand() : iconify();

  QGraphicsItem::mouseDoubleClickEvent(a_event);
}

void Node::setElement(elements::Element *const a_element)
{
  if (m_element) qDebug() << "Already have element!";

  m_element = a_element;

  auto const &inputs = m_element->inputs();
  auto const &outputs = m_element->outputs();

  switch (m_type) {
    case Type::eElement:
      for (size_t i = 0; i < inputs.size(); ++i) {
        QString const name = QString::fromStdString(inputs[i].name);
        addSocket(SocketType::eInput, static_cast<uint8_t>(i), name, inputs[i].type);
      }
      for (size_t i = 0; i < outputs.size(); ++i) {
        QString const name = QString::fromStdString(outputs[i].name);
        addSocket(SocketType::eOutput, static_cast<uint8_t>(i), name, outputs[i].type);
      }
      break;
    case Type::eInputs:
      for (size_t i = 0; i < inputs.size(); ++i) {
        QString const name = QString::fromStdString(inputs[i].name);
        addSocket(SocketType::eOutput, static_cast<uint8_t>(i), name, inputs[i].type);
      }
      break;
    case Type::eOutputs:
      for (size_t i = 0; i < outputs.size(); ++i) {
        QString const name = QString::fromStdString(outputs[i].name);
        addSocket(SocketType::eInput, static_cast<uint8_t>(i), name, outputs[i].type);
      }
      break;
  }

  m_element->onChange([&](elements::Element *const a_changed) {
    auto const &changedOutputs = a_changed->outputs();
    for (size_t i = 0; i < changedOutputs.size(); ++i) {
      switch (changedOutputs[i].type) {
        case ValueType::eBool: {
          bool const signal{ std::get<bool>(changedOutputs[i].value) };
          m_outputs[static_cast<int>(i)]->setSignal(signal);
          break;
        }
        default: break;
      }
    }
  });

  calculateBoundingRect();
}

void Node::setIcon(QString a_icon)
{
  m_icon.load(a_icon);
}

void Node::iconify()
{
  m_mode = Mode::eIconified;

  for (auto &&input : m_inputs) input->hideName();
  for (auto &&output : m_outputs) output->hideName();

  prepareGeometryChange();
  calculateBoundingRect();
}

void Node::expand()
{
  m_mode = Mode::eExpanded;

  for (auto &&input : m_inputs) input->showName();
  for (auto &&output : m_outputs) output->showName();

  prepareGeometryChange();
  calculateBoundingRect();
}

void Node::addSocket(SocketType const a_type, uint8_t const a_id, QString const a_name, ValueType const a_valueType)
{
  auto *const socket{ new SocketItem{ a_type, this } };
  socket->setElementId(m_element->id());
  socket->setSocketId(a_id);

  switch (a_valueType) {
    case ValueType::eBool: socket->setColors(get_color(Color::eBoolSignalOff), get_color(Color::eBoolSignalOn)); break;
    case ValueType::eFloat:
      socket->setColors(get_color(Color::eFloatSignalOff), get_color(Color::eFloatSignalOn));
      break;
    case ValueType::eInt:
      socket->setColors(get_color(Color::eIntegerSignalOff), get_color(Color::eIntegerSignalOn));
      break;
  }

  socket->setName(a_name);
  if (a_type == SocketType::eInput)
    m_inputs.push_back(socket);
  else
    m_outputs.push_back(socket);
}

void Node::calculateBoundingRect()
{
  auto const inputsCount = m_inputs.count();
  auto const outputsCount = m_outputs.count();
  auto const socketsCount = std::max(inputsCount, outputsCount);

  constexpr int32_t const SPACING_ICONIFIED = 5;
  constexpr int32_t const SPACING_EXPANED = 15;
  constexpr int32_t const SOCKET_SIZE = SocketItem::SIZE;
  //  int32_t const horizontalAdjust = 0;

  QSizeF size{};

  int32_t spacing{};
  switch (m_mode) {
    case Mode::eIconified: {
      spacing = SPACING_ICONIFIED;
      if (socketsCount < 2)
        size = ICON_SIZE;
      else {
        size.rwidth() = ICON_SIZE.width();
        size.rheight() = ROUND_FACTOR + ROUND_FACTOR;
        size.rheight() += socketsCount * SOCKET_SIZE + (socketsCount - 1) * spacing;
      }
      break;
    }
    case Mode::eExpanded: {
      spacing = SPACING_EXPANED;
      size = ICON_SIZE;
      size.rheight() += 50;
      size.rwidth() += 100;
      break;
    }
  }

  QRectF const rect{ QPointF{ -size.width() / 2.0, -size.height() / 2.0 }, size };

  auto const inputsHeight = inputsCount * SOCKET_SIZE + (inputsCount - 1) * spacing;
  auto const outputsHeight = outputsCount * SOCKET_SIZE + (outputsCount - 1) * spacing;

  qreal offset{ -inputsHeight / 2.0 };
  for (auto &&input : m_inputs) {
    input->setPos(-rect.width() / 2.0, offset);
    offset += spacing + SOCKET_SIZE;
  }

  offset = -outputsHeight / 2.0;
  for (auto &&output : m_outputs) {
    output->setPos(rect.width() / 2.0, offset);
    offset += spacing + SOCKET_SIZE;
  }

  m_boundingRect = rect;

  update();
}

} // namespace nodes