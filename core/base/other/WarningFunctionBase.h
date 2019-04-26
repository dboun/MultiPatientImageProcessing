#ifndef WARNING_FUNCTION_BASE_H
#define WARNING_FUNCTION_BASE_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "DataManager.h"
#include "DataViewBase.h"

/** This class is meant to be used for functions before running an algorithm.
 *  It is to be used as a warning/guard system, to check if all the creteria for running
 *  an algorithm are met. */
class WarningFunctionBase : public QObject
{
    Q_OBJECT

public:
    WarningFunctionBase(QObject* parent = nullptr);

    ~WarningFunctionBase();

    virtual void SetDataManager(DataManager* dataManager);

    virtual void SetDataView(DataViewBase* dataView);

    QString GetName(); 

    /** This is provided, but it's best to listen for OperationAllowanceChanged */
    bool IsOperationAllowed();

    QString GetErrorMessageIfNotAllowed();

public slots:
    // Slots for DataViewBase
	virtual void SelectedSubjectChangedHandler(long uid);
	virtual void SelectedDataChangedHandler(long iid);
	virtual void DataAddedForSelectedSubjectHandler(long iid);
	virtual void DataRemovedFromSelectedSubjectHandler(long iid);
	virtual void DataCheckedStateChangedHandler(long iid, bool checkState);

    // Slots for DataManager (Useful only if an implementation concerns all subjects)
	void SubjectAddedHandler(long uid);
	void SubjectRemovedHandler(long uid);
	void SubjectDataChangedHandler(long uid);

signals:
    void OperationAllowanceChanged(WarningFunctionBase* function, bool allow,
        QString errorMessageIfNotAllowed
    );
    void NewWarning(WarningFunctionBase* function,
        QString warning
    );
    void WarningWasRemoved(WarningFunctionBase* function,
        QString warningThatWasRemoved
    );

protected:
    // Implementations of this class, should call the below methods to update their state
    void SetName(QString name);
    void SetOperationAllowed(bool allowed,
        QString GetErrorMessageIfNotAllowed = ""
    );
    /** This was call WarningWasRemoved if an old warning isn't in newWarnings */
    void UpdateWarnings(QStringList newWarnings);

    DataManager*           m_DataManager;
    DataViewBase*          m_DataView;

private:
    QString                m_Name, m_ErrorMessage;
    QStringList            m_Warnings;
    bool                   m_OperationAllowed = true;
};

#endif // ! WARNING_FUNCTION_BASE_H