#ifndef ALGORITHM_MODULE_BASE_H
#define ALGORITHM_MODULE_BASE_H

#include <QObject>
#include <QString>

#include "DataManager.h"
#include "DataViewBase.h"

class AlgorithmModuleBase : public QObject
{
  Q_OBJECT

public:

  enum SEVERITY {
    LOW, MEDIUM, HIGH
  };

  explicit AlgorithmModuleBase(QObject *parent = nullptr) : QObject(parent) {}
  ~AlgorithmModuleBase() {}

  /** These should be set by the scheduler */
  void SetDataManager(DataManager* dataManager);
  void SetDataView(DataViewBase* dataView);
  void SetUid(long uid);

  /** Getters */
  long          GetUid();
  QString       GetAlgorithmName();
  QString       GetAlgorithmNameShort();
  SEVERITY      GetSeverity();
  bool          IsRunning();
  bool          IsCanceled();


  /** This is called by the scheduler to run an algorithm */
  void Run(); 

  /** Override this to add functionality */
  virtual void Algorithm();

signals:
  void ProgressUpdateUI(long uid, QString message, int progress);
  void AlgorithmFinished(AlgorithmModuleBase* algorithm);
  void AlgorithmFinishedWithError(AlgorithmModuleBase* algorithm, QString errorMessage);

protected:
  DataManager*  GetDataManager();
  DataViewBase* GetDataView();

  void          SetAlgorithmName(QString algorithmName);
  void          SetAlgorithmNameShort(QString algorithmNameShort);
  void          SetSeverity(SEVERITY severity);

  DataManager*  m_DataManager;
  DataViewBase* m_DataView;
  long          m_Uid = -1;
  bool          m_IsRunning = false;
  bool          m_CancelFlag = false;

  QString m_AlgorithmName      = "Full algorithm name";
  QString m_AlgorithmNameShort = "Short algorithm name";
  SEVERITY m_Severity          = SEVERITY::MEDIUM;
};

#endif // ! ALGORITHM_MODULE_BASE_H
