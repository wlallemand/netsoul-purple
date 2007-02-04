%define rpmrelease 0
Summary:    Netsoul protocol plugin for Gaim
Name:       gaim-netsoul
Version:    0.2.1
Release:    %{gaimver}%{?gaimdist:.%{gaimdist}}.%{rpmrelease}
License:    GPL
Group:      Applications/Internet
Url:        http://www.bilboed.com/netsoul/
Source:     %{name}-%{version}.tar.gz
BuildRoot:  %{_tmppath}/%{name}-%{version}-root

Requires:   gaim = 1:%{gaimver}
BuildRequires: pkgconfig, libtool, gaim-devel

%description
gaim-netsoul is a protocol plug-in for Gaim that allows using the netsoul IM service.

To rebuild for a specific Gaim version or dist tag:
rpmbuild --rebuild gaim-netsoul-0.2.1-%{rpmrelease}.src.rpm --define 'gaimver %{gaimver}' --define 'gaimdist fc1'

%prep
%setup -q

%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{_prefix}

make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT%{_prefix} bindir=$RPM_BUILD_ROOT%{_bindir} \
     datadir=$RPM_BUILD_ROOT%{_datadir} includedir=$RPM_BUILD_ROOT%{_includedir} \
     libdir=$RPM_BUILD_ROOT%{_libdir} mandir=$RPM_BUILD_ROOT%{_mandir} \
     sysconfdir=$RPM_BUILD_ROOT%{_sysconfdir} \
     install

strip $RPM_BUILD_ROOT%{_libdir}/gaim/*.so || :
rm -f $RPM_BUILD_ROOT%{_libdir}/gaim/libnetsoul.la $RPM_BUILD_ROOT%{_libdir}/gaim/libnetsoul.a

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)

%doc README ChangeLog
%{_libdir}/gaim/libnetsoul.so
%{_datadir}/pixmaps/gaim/status/default/*

%changelog
* Mon Jul 19 2004 Stu Tomlinson <stu@nosnilmot.com>
- Initial spec file
