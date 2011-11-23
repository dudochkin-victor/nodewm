Summary: wmsigner module for Webmoney signing data
Name: wmsigner
Version: 2.0.2
Release: 1
Group: Applications/Tools
License: BSD
Source: %{name}-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-root
Requires: gcc-c++
%description
wmsigner module for Webmoney signing data

%prep
%setup -q

%build
#./configure --prefix=/usr/local/bin
make

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}
make install prefix=%{buildroot}

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%attr(0755, root, bin) /usr/local/bin/wmsigner
%attr(0755, root, bin) /usr/local/bin/code64
%attr(0644, root, bin) /usr/local/man/man1/wmsigner.1
%attr(0644, root, bin) /usr/share/doc/wmsigner/ChangeLog
%attr(0644, root, bin) /usr/share/doc/wmsigner/INSTALL
%attr(0644, root, bin) /usr/share/doc/wmsigner/README
%attr(0644, root, bin) /usr/share/doc/wmsigner/README.rus

%changelog
* Mon Sep 21 2009 Alexander Oryol <eagle.alex@gmail.com> 2.0.2
- update to 2.0.2
* Thu Aug 14 2007 Alexander Oryol <eagle.alex@gmail.com> 2.0.1
- update to 2.0.1
* Mon Jul 23 2007 Alexander Oryol <eagle.alex@gmail.com> 2.0
- update to 2.0
* Wed Jul 18 2007 Alexander Oryol <eagle.alex@gmail.com> 2.0-beta
- update to 2.0beta
* Wed Jul 05 2006 Alexander Oryol <eagle.alex@gmail.com> 1.2-1
- update to 1.2
- add ChangeLog, README, README.rus in /usr/share/doc/WMSigner to RPM
