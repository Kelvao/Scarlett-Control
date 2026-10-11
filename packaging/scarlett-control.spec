%define _build_id_links none
%global __os_install_post %{nil}
%define __strip /bin/true

Name:           scarlett-control
Version:        %{?pkg_version}%{!?pkg_version:1.0.0}
Release:        1%{?dist}
Summary:        Controle de hardware e system tray para Focusrite Scarlett 2i2
License:        GPL-2.0-or-later
URL:            https://github.com/kelvao/scarlett-control

%description
Controle em C para Focusrite Scarlett 2i2 em qualquer ambiente Linux.
Suporta alternância de nível (Line/Inst), modo Air, Phantom Power (+48V)
e monitoramento direto (Off/Mono/Stereo) via terminal e bandeja do sistema.

%install
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_datadir}/applications
mkdir -p %{buildroot}%{_datadir}/icons/hicolor/scalable/apps
mkdir -p %{buildroot}%{_sysconfdir}/xdg/autostart
mkdir -p %{buildroot}%{_datadir}/scarlett-control/locales

install -m 755 %{_sourcedir}/scarlett-control %{buildroot}%{_bindir}/scarlett-control
install -m 644 %{_sourcedir}/scarlett-control.desktop %{buildroot}%{_datadir}/applications/scarlett-control.desktop
install -m 644 %{_sourcedir}/scarlett-control.desktop %{buildroot}%{_sysconfdir}/xdg/autostart/scarlett-control.desktop
install -m 644 %{_sourcedir}/assets/scarlett-control.svg %{buildroot}%{_datadir}/icons/hicolor/scalable/apps/scarlett-control.svg
install -m 644 %{_sourcedir}/locales/*.json %{buildroot}%{_datadir}/scarlett-control/locales/

%files
%{_bindir}/scarlett-control
%{_datadir}/applications/scarlett-control.desktop
%{_sysconfdir}/xdg/autostart/scarlett-control.desktop
%{_datadir}/icons/hicolor/scalable/apps/scarlett-control.svg
%{_datadir}/scarlett-control

%changelog
* Sun Oct 11 2026 Developer <dev@localhost> - 1.0.0-1
- Initial RPM release

