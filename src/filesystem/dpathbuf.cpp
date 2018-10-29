#include "dpathbuf.h"

/*!
 * \class DPathBuf
 * \brief DPathBuf cat path friendly and supoort multiplatform.
 */

/*!
 * \brief Create DPathBuf from a string.
 * \param path
 */
Dtk::Core::DPathBuf::DPathBuf(const QString &path)
{
    m_path = QDir(path).absolutePath();
}
