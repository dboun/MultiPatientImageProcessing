#include "WarningFunctionBase.h"

WarningFunctionBase::WarningFunctionBase(QObject* parent) : 
    QObject(parent),
    m_Name("Warning Function") // This needs to be set this manually later
{

}

WarningFunctionBase::~WarningFunctionBase()
{

}

void WarningFunctionBase::SetDataManager(DataManager* dataManager)
{
    m_DataManager = dataManager;

    connect(m_DataManager, SIGNAL(SubjectAdded(long)), 
        this, SLOT(SubjectAddedHandler(long))
    );
	connect(m_DataManager, SIGNAL(SubjectRemoved(long)), 
        this, SLOT(SubjectRemovedHandler(long))
    );
	connect(m_DataManager, SIGNAL(SubjectDataChanged(long)), 
        this, SLOT(SubjectDataChangedHandler(long))
    );
}

void WarningFunctionBase::SetDataView(DataViewBase* dataView)
{
    m_DataView = dataView;

    connect(m_DataView, SIGNAL(SelectedSubjectChanged(long)), 
        this, SLOT(SelectedSubjectChangedHandler(long))
    );
	connect(m_DataView, SIGNAL(SelectedDataChanged(long)), 
        this, SLOT(SelectedDataChangedHandler(long))
    );
	connect(m_DataView, SIGNAL(DataAddedForSelectedSubject(long)), 
        this, SLOT(DataAddedForSelectedSubjectHandler(long))
    );
	connect(m_DataView, SIGNAL(DataRemovedFromSelectedSubject(long)), 
        this, SLOT(DataRemovedFromSelectedSubjectHandler(long))
    );
	connect(m_DataView, SIGNAL(DataCheckedStateChanged(long, bool)), 
        this, SLOT(DataCheckedStateChangedHandler(long, bool))
    );
}

QString WarningFunctionBase::GetName()
{
    return m_Name;
}

bool WarningFunctionBase::IsOperationAllowed()
{
    return m_OperationAllowed;
}

QString WarningFunctionBase::GetErrorMessageIfNotAllowed()
{
    if (m_OperationAllowed) 
    {
        m_ErrorMessage = "";
        return "";
    }
    return m_ErrorMessage;
}

void WarningFunctionBase::SelectedSubjectChangedHandler(long uid)
{

}

void WarningFunctionBase::SelectedDataChangedHandler(long iid)
{

}

void WarningFunctionBase::DataAddedForSelectedSubjectHandler(long iid)
{

}

void WarningFunctionBase::DataRemovedFromSelectedSubjectHandler(long iid)
{

}

void WarningFunctionBase::DataCheckedStateChangedHandler(long iid, bool checkState)
{

}

void WarningFunctionBase::SubjectAddedHandler(long uid)
{

}

void WarningFunctionBase::SubjectRemovedHandler(long uid)
{

}

void WarningFunctionBase::SubjectDataChangedHandler(long uid)
{

}

void WarningFunctionBase::SetName(QString name)
{
    m_Name = name;
}

void WarningFunctionBase::SetOperationAllowed(bool allowed, 
    QString errorMessageIfNotAllowed)
{
    if (allowed) 
    { 
        errorMessageIfNotAllowed = ""; 
    }

    if (m_OperationAllowed != allowed)
    {
        m_OperationAllowed = allowed;
        emit OperationAllowanceChanged(this, 
            m_OperationAllowed, 
            errorMessageIfNotAllowed
        );
    }
}

void WarningFunctionBase::UpdateWarnings(QStringList newWarnings)
{
    std::vector<QString> updateAdded, updateRemoved;

    for (const QString& nw : newWarnings)
    {
        if (!m_Warnings.contains(nw))
        {
            updateAdded.push_back(nw);
        }
    }

    for (const QString& ow : m_Warnings)
    {
        if (!newWarnings.contains(ow))
        {
            updateRemoved.push_back(ow);
        }
    }

    m_Warnings = newWarnings;

    for (const QString& w : updateAdded)
    {
        emit NewWarning(this, w);
    }

    for (const QString& w : updateRemoved)
    {
        emit WarningWasRemoved(this, w);
    }
}