#ifndef APPLICATION_BASE_H
#define APPLICATION_BASE_H

#include <QObject>

class ApplicationBase : public QObject
{
  Q_OBJECT

public:
  explicit ApplicationBase(QObject *parent = nullptr) : QObject(parent) {}
  ~ApplicationBase() {}

  void SetUid(long uid);
  long GetUid();

  void EmitProgressUpdateForDebugging(int progress = 100);

signals:
  void ProgressUpdateUI(long uid, QString message, int progress);

protected:
  long uid;
};

#endif // APPLICATION_BASE_H
