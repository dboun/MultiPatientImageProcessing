#ifndef CustomQmitkStdMultiWidget_h
#define CustomQmitkStdMultiWidget_h

#include "MitkQtWidgetsExports.h"

#include <mitkDataStorage.h>
#include <mitkLogoAnnotation.h>

#include <mitkMouseModeSwitcher.h>

#include <QFrame>
#include <qsplitter.h>
#include <qwidget.h>

#include <QmitkLevelWindowWidget.h>
#include <QmitkRenderWindow.h>

#include <mitkBaseRenderer.h>

#include <QmitkStdMultiWidget.h>

class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class QSpacerItem;
class QmitkLevelWindowWidget;
class QmitkRenderWindow;
class vtkCornerAnnotation;
class vtkMitkRectangleProp;

namespace mitk
{
  class RenderingManager;
}

class CustomQmitkStdMultiWidget : public QmitkStdMultiWidget
{
  Q_OBJECT

public:
  CustomQmitkStdMultiWidget(
    QWidget *parent = nullptr,
    Qt::WindowFlags f = nullptr,
    mitk::RenderingManager *renderingManager = nullptr,
    mitk::BaseRenderer::RenderingMode::Type renderingMode = mitk::BaseRenderer::RenderingMode::Standard,
    const QString &name = "stdmulti"
  );
  
  ~CustomQmitkStdMultiWidget() override;

  // Call this after initializing
  void AddSlidersToViews(
    mitk::RenderingManager *renderingManager = nullptr,
    mitk::BaseRenderer::RenderingMode::Type renderingMode = mitk::BaseRenderer::RenderingMode::Standard,
    const QString &name = "stdmulti"
  );

public slots:
  // This is always called after OnLayoutDesignChanged
  void CustomOnLayoutDesignChanged(int layoutDesignIndex);

};
#endif /*CustomQmitkStdMultiWidget_h*/
