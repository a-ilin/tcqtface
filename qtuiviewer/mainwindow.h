/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2019 by Aleksei Ilin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFrame>
#include <QPointer>

#include <wlx_interfaces.h>

class ProxyWidget : public QFrame
{
public:
  ProxyWidget(QWidget* parent);

  QString load(const QString& filePath);
  QWidget* uiWidget() const { return m_uiWidget; }

protected:
  QWidget* m_uiWidget;
};

class MainWindow : public QWidget, public IAbstractWlxWindow
{
  Q_OBJECT

public:
  MainWindow(IParentWlxWindow* parent);

  void load(const QString& file, int showFlags) Q_DECL_OVERRIDE;
  void reload() Q_DECL_OVERRIDE;

  int print(const QString& file, const QString& printer, int flags, const QMarginsF& margins) Q_DECL_OVERRIDE;

protected:
  QString m_filePath;
  int m_showFlags;

  QPointer<ProxyWidget> m_proxy;
};

#endif // MAINWINDOW_H
