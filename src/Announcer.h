/***************************************************************************
 *   Copyright 2019-2022 Leonard de Ruijter, James Teh - OSARA             *
 *   Copyright 2017 The Qt Company Ltd.                                    *
 *   Copyright (C) 2022 by Vadim Peretokin - vadim.peretokin@mudlet.org    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef ANNOUNCER_H
#define ANNOUNCER_H

#include <QAccessible>
#include <QAccessibleInterface>
#include <QAccessibleWidget>
#include <QObject>
#include <QWidget>
#if defined(Q_OS_WIN32)
#include <QLibrary>
#endif

#if defined(Q_OS_LINUX)
// workaround AT-SPI2's inability to simply read out text with a fake statusbar and a notification
// see https://mail.gnome.org/archives/orca-list/2022-June/msg00027.html
class InvisibleNotification : public QWidget {
  Q_OBJECT

public:
  Q_DISABLE_COPY(InvisibleNotification)
  explicit InvisibleNotification(QWidget *parent);

  void setText(const QString &text);
  QString text();

private:
  QString mText;
};

class InvisibleStatusbar : public QWidget {
  Q_OBJECT

public:
  Q_DISABLE_COPY(InvisibleStatusbar)
  explicit InvisibleStatusbar(QWidget *parent);
};

class InvisibleAccessibleNotification : public QAccessibleWidget {
public:
  explicit InvisibleAccessibleNotification(QWidget *w)
      : QAccessibleWidget(w, QAccessible::Role::Notification) {}

private:
  InvisibleNotification *notification() const;

protected:
  QString text(QAccessible::Text t) const override;
};

class FakeAccessibleStatusbar : public QAccessibleWidget {
public:
  explicit FakeAccessibleStatusbar(QWidget *w)
      : QAccessibleWidget(w, QAccessible::Role::StatusBar) {}
};
#endif

class Announcer : public QWidget {
  Q_OBJECT
public:
  Q_DISABLE_COPY_MOVE(Announcer)
  explicit Announcer(QWidget *parent = nullptr);
  void announce(const QString text);

#if defined(Q_OS_LINUX)
  static QAccessibleInterface *accessibleFactory(const QString &classname,
                                                 QObject *object) {
#undef interface // mingw compilation breaks without this
    QAccessibleInterface *interface = nullptr;

    if (classname == QLatin1String("InvisibleNotification") && object &&
        object->isWidgetType()) {
      interface =
          new InvisibleAccessibleNotification(static_cast<QWidget *>(object));
    } else if (classname == QLatin1String("InvisibleStatusbar") && object &&
               object->isWidgetType()) {
      interface = new FakeAccessibleStatusbar(static_cast<QWidget *>(object));
    }

    return interface;
  }
#endif

private:
#if defined(Q_OS_LINUX)
  InvisibleNotification *notification;
  InvisibleStatusbar *statusbar;
#endif

#if defined(Q_OS_WIN)
  QScopedPointer<QLibrary> mpUiaLibrary;

  bool initializeUia();

  class UiaProvider;
  UiaProvider *uiaProvider{};
#endif
};
#endif // ANNOUNCER_H
