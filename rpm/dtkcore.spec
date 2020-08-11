Name:           dtkcore
Version:        5.2.2.3
Release:        1%{?dist}
Summary:        Deepin tool kit core modules
License:        GPLv3
URL:            https://github.com/linuxdeepin/dtkcore
Source0:        %{name}_%{version}.orig.tar.xz
BuildRequires:  gcc-c++
BuildRequires:  annobin
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(gsettings-qt)
Obsoletes:      deepin-tool-kit <= 0.3.3
Obsoletes:      deepin-tool-kit-devel <= 0.3.3
Obsoletes:      dtksettings <= 0.1.7
Obsoletes:      dtksettings-devel <= 0.1.7

%description
Deepin tool kit core modules.

%package devel
Summary:        Development package for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       qt5-qtbase-devel

%description devel
Header files and libraries for %{name}.

%prep
%setup -q

sed -i 's|/lib|/libexec|' tools/settings/settings.pro
## consider relying on %%_qt5_bindir (see %%build below) instead of patching -- rex
#sed -i 's|qmake|qmake-qt5|' src/dtk_module.prf
#sed -i 's|lrelease|lrelease-qt5|' tools/script/dtk-translate.py src/dtk_translation.prf

%build
# help find (and prefer) qt5 utilities, e.g. qmake, lrelease
export PATH=%{_qt5_bindir}:$PATH
%qmake_qt5 PREFIX=%{_prefix} \
           DTK_VERSION=%{version} \
           LIB_INSTALL_DIR=%{_libdir} \
           BIN_INSTALL_DIR=%{_libexecdir}/dtk5 \
           TOOL_INSTALL_DIR=%{_libexecdir}/dtk5
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}

%ldconfig_scriptlets

%files
%doc README.md
%license LICENSE
%{_libdir}/libdtkcore.so.*
%{_libexecdir}/dtk5/dtk-settings
%{_libexecdir}/dtk5/dtk-license.py*
%{_libexecdir}/dtk5/dtk-translate.py*
%{_libexecdir}/dtk5/deepin-os-release

%files devel
%doc doc/Specification.md
%{_includedir}/libdtk-*/
%{_qt5_archdatadir}/mkspecs/features/*.prf
%{_qt5_archdatadir}/mkspecs/modules/*.pri
%{_libdir}/cmake/Dtk/
%{_libdir}/cmake/DtkCore/
%{_libdir}/cmake/DtkCMake/
%{_libdir}/cmake/DtkTools/
%{_libdir}/pkgconfig/dtkcore.pc
%{_libdir}/libdtkcore.so
/usr/share/glib-2.0/schemas/*

%changelog
* Thu Jun 11 2020 uoser <uoser@uniontech.com> - 5.2.2.3
- Update to 5.2.2.3
