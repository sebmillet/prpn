Name:           prpn
Version:        0.5.2
Release:        1
Summary:        A Reverse Polish Notation calculator
Group:          Applications/Engineering
License:        GPLv3+
URL:            http://prpn.origo.ethz.ch
Source0:        http://download.origo.ethz.ch/prpn/1970/prpn-0.5.2.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:  wxGTK-devel >= 2.8, gettext, desktop-file-utils
Requires:       wxGTK >= 2.8
%description
pRPN is an RPN calculator based on WxWidgets for the GUI, that provides the
following objects: Reals, Complex, Vectors, Matrices, Binaries, Lists,
Expressions, Strings and Programs.
pRPN also provides a terminal interface, to be used interactively or for
batch processing.

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
%find_lang %{name}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%files -f %{name}.lang
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_bindir}/%{name}c
%{_datadir}/man/*
%{_docdir}/%{name}/README
%{_docdir}/%{name}/prpnen.html
%{_docdir}/%{name}/prpnen.txt
%{_docdir}/%{name}/prpnfr.html
%{_docdir}/%{name}/prpnfr.txt
%{_datadir}/applications/prpn.desktop
%{_datadir}/pixmaps/prpn.png
%doc AUTHORS ChangeLog COPYING INSTALL NEWS README

%changelog
* Tue Mar 29 2011 Sébastien Millet <sebastien.millet1@club-internet.fr>
- Initial spec for prpn-0.5.2

