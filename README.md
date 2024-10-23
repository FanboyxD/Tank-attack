# Tank-attack
Autor: Fabricio Mena Mejia y Harold Madriz Cerdas   
Profesor: Luis Alonso Barboza Artavia  
Curso: Datos II
## Requisitos
* Sistema operativo: Ubuntu 24.04.1 LTS
* Editor de codigo: CLion
* Este codigo fue desarrollado en ubuntu por ende es recomendable utilizar este sistema operativo para compilar el proyecto.
* La version de cmake utilizada para el proyecto es la `3.29`, asi que si presenta errores la version actual puede actualizarla mediante los siguientes comandos
  * `sudo apt update`
  * `sudo apt install software-properties-common`
  * `sudo add-apt-repository ppa:kitware/release`
  * `sudo apt update`
  * `sudo apt install cmake` 
* Este juego fue creado utilizando la biblioteca grafica SFML, asi que hay que descargarla mediante el siguiente comando `sudo apt-get install libsfml-dev`.
* Una vez instalada la libreria hay que agregar sus componentes necesarios en el `CMakeLists.txt` mediante `find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)` y `target_link_libraries(Proyecto2 sfml-graphics sfml-window sfml-system)`.

# Compilacion y Ejecucion  
Hay que realizar la build del CMakeLists.txt, una vez realizada esta simplemente se corre el main.cpp, hay que tener en cuenta que el CMakeLists.txt debe incluir los header ademas del ejecutable, de la siguiente forma:  
add_executable(Proyecto2 main.cpp
        Matrix.h
        Player.h
        Bullet.h
)
