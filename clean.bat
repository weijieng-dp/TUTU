
cmake --build build --target clean

if exist build (
  rmdir /s /q build
)
if exist android (
  if exist android\.gradle (
    echo Removing android\.gradle
    rmdir /s /q android\.gradle
  )
  if exist android\.idea (
    echo Removing android\.idea
    rmdir /s /q android\.idea
  )
  if exist android\build (
    echo Removing android\build
    rmdir /s /q android\build
  )
  if exist android\app\.cxx (
    echo Removing android\app\.cxx
    rmdir /s /q android\app\.cxx
  )
  if exist android\app\build (
    echo Removing android\app\build
    rmdir /s /q android\app\build
  )
)
Pause