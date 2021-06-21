type src\Fast-DDS-statistics-backend\.github\workflows\colcon_windows.meta
colcon build --build-base build_%1 --install-base install_%1 --event-handlers=console_direct+ --executor sequential --metas src/Fast-DDS-statistics-backend/.github/workflows/colcon_windows.meta --cmake-args -DCMAKE_BUILD_TYPE=%1 -Ax64 -T host=x64
