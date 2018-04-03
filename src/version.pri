isEmpty(VERSION) {
    !isEqual(TARGET, dtkcore) {
        PKG_CONFIG = $$pkgConfigExecutable()
        !isEmpty(PKG_CONFIG): VERSION = $$system($$PKG_CONFIG --modversion dtkcore)
    }

    isEmpty(VERSION): VERSION = $$system(git -C $$_PRO_FILE_PWD_ describe --tags --abbrev=0)
    isEmpty(VERSION): VERSION = $$DTK_VERSION
    isEmpty(VERSION): error(VERSION is empty)
    VERSION = $$replace(VERSION, [^0-9.],)
}

ver_list = $$split(VERSION, .)

isEmpty(VER_MAJ) {
    VER_MAJ = $$first(ver_list)
}

isEmpty(VER_MIN) {
    VER_MIN = $$member(ver_list, 1, 1)
    isEmpty(VER_MIN):VER_MIN = 0
}

isEmpty(VER_PAT) {
    VER_PAT = $$member(ver_list, 2, 2)
    isEmpty(VER_PAT):VER_PAT = 0
}

isEmpty(VER_BUI) {
    VER_BUI = $$member(ver_list, 3, 3)
    isEmpty(VER_BUI):VER_BUI = 0
}
