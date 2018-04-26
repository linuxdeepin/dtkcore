#include "dpathbuf.h"

Dtk::Core::DPathBuf::DPathBuf(const QString &path)
{
    m_path = QDir(path).absolutePath();
}
