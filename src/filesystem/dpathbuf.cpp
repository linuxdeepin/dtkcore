#include "dpathbuf.h"

/*!
 * \~english \class Dtk::Core::DPathBuf
 * \brief Dtk::Core::DPathBuf cat path friendly and supoort multiplatform.
 */

/*!
 * \~chinese \class Dtk::Core::DPathBuf
 * \brief Dtk::Core::DPathBuf是一个用于跨平台拼接路径的辅助类。
 * 它能够方便的写出链式结构的路径拼接代码。
```
DPathBuf logPath(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
logPath = logPath / ".cache" / "deepin" / "deepin-test-dtk" / "deepin-test-dtk.log";
```
 */


DCORE_BEGIN_NAMESPACE

/*!
 * \brief Create Dtk::Core::DPathBuf from string.
 * \param path
 */
DPathBuf::DPathBuf(const QString &path)
{
    m_path = QDir(path).absolutePath();
}

DPathBuf::DPathBuf()
    : DPathBuf(QString())
{

}

DCORE_END_NAMESPACE
