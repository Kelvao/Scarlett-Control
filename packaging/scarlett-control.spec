%define _build_id_links none
%global __os_install_post %{nil}
%define __strip /bin/true

Name:           scarlett-control
Version:        %{?pkg_version}%{!?pkg_version:1.0.0}
Release:        1%{?dist}
Summary:        Hardware control and system tray indicator for Focusrite Scarlett 2i2
License:        MIT
URL:            https://github.com/Kelvao/Scarlett-Control

%description
High-performance, lightweight C utility for Focusrite Scarlett 2i2 interfaces
on Linux. Supports input impedance switching (Line/Inst), Air mode, Phantom Power (+48V),
and direct monitoring (Off/Mono/Stereo) via both CLI and system tray.

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
- Initial release
