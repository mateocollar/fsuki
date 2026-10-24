# fsuki

Librería de sistema de archivos simple y multiplataforma para [itsuki-lang](https://github.com/Apolo1061/itsuki-lang).

![Versión](https://img.shields.io/badge/versi%C3%B3n-1.0.0-blue)
![Estado](https://img.shields.io/badge/estado-experimental-orange)
![Plataformas](https://img.shields.io/badge/plataformas-Windows%20%7C%20Linux-lightgrey)
![Arquitectura](https://img.shields.io/badge/arquitectura-x86__32%20%7C%20x86__64-brightgreen)
![Itsuki](https://img.shields.io/badge/itsuki-5.5.0%2B-9cf)
![Licencia](https://img.shields.io/badge/licencia-MIT-green)
[![Discord](https://img.shields.io/badge/Discord-Unirse-7289DA?logo=discord&logoColor=white)](https://discord.gg/K6SEpB7Pf5)

---

## Descripción

**fsuki** es una librería de sistema de archivos para **itsuki-lang** que abstrae las diferencias entre Windows y Linux (POSIX) bajo una API única. Cubre lo que `fundamental.huki` no trae: creación de directorios anidados, listado de carpetas, copia de archivos, renombrado y utilidades de rutas (`join`, `basename`, `dirname`, `ext`).

> **Estado:** fase experimental. La API puede cambiar sin previo aviso.
> **Compatibilidad:** Itsuki 5.5.0 o superior, arquitecturas x86_32 y x86_64.
> **Convención de rutas:** separador `/` en ambas plataformas (Windows lo acepta en todas sus APIs).

---

## Características

- **Multiplataforma:** mismo código fuente para Windows y Linux.
- **Directorios anidados:** `fs_mkdirs()` crea toda la ruta de una (`mkdir -p`).
- **Listado de directorios:** `fs_list()` y `fs_count()` sin `.` ni `..`.
- **Archivos:** copia binaria, `touch`, eliminación y renombrado.
- **Utilidades de rutas:** `fs_join`, `fs_basename`, `fs_dirname`, `fs_ext`.
- **Entorno:** directorio de trabajo actual (`fs_cwd`) y cambio (`fs_chdir`).
- **Sin inicialización:** no necesita `init`/`cleanup` como los sockets.
- **Códigos de error negativos** documentados por función.

Lectura y escritura de contenido: usá `file_read_all`, `file_write_all` y `file_append` de `fundamental.huki` — fsuki no duplica lo que el estándar ya cubre.

---

## Instalación

### Vía gestor de paquetes de Itsuki (recomendado)

~~~bash
itsuki install mateocollar/fsuki
~~~

El paquete queda disponible globalmente en `~/.itsuki/packages/`.

### Manual

Copiá `fsuki.huki` y `fsuki.c` en tu proyecto. No requiere librerías externas en ninguna plataforma.

---

## Uso básico

~~~itsuki
#include <fundamental.huki>
#include <fsuki.huki>

fn main() -> int {
    // Crear una estructura anidada de una
    if (fs_mkdirs("datos/configs/perfil") != 0) {
        return 1;
    }

    // Listar un directorio
    var listado: char[256];
    var n: int = fs_list("datos", listado, 256);
    printf("%d entradas:\n%s\n", n, listado);

    // Rutas
    var ruta: char[256];
    fs_join("datos/configs", "usuario.json", ruta, 256);

    var nombre: char[128];
    fs_basename(ruta, nombre, 128);   // "usuario.json"

    var ext: char[32];
    fs_ext(ruta, ext, 32);            // "json"

    // Copiar y eliminar
    file_write_all(ruta, "{\"nombre\": \"itsuki\"}");
    fs_copy(ruta, "datos/configs/copia.json");
    fs_remove(ruta);

    return 0;
}
~~~

### Compilación con Itsuki

~~~bash
# NO pases fsuki.c: Itsuki lo encuentra solo
itsuki run main.suki
~~~

---

## API

### Metadatos

| Función | Descripción |
|---|---|
| `fs_exists(path)` | 1 si la ruta existe, 0 si no. |
| `fs_is_file(path)` | 1 si es archivo regular, 0 si no. |
| `fs_size(path)` | Tamaño en bytes, o -1. |

### Directorios

| Función | Descripción |
|---|---|
| `fs_mkdir(path)` | Crea un directorio (el padre debe existir). |
| `fs_mkdirs(path)` | Crea la ruta completa (`mkdir -p`); ignora niveles existentes. |
| `fs_rmdir(path)` | Elimina un directorio vacío. |
| `fs_list(path, buf, len)` | Nombres separados por `\n`. Retorna cantidad, -1 error, -2 buffer chico. |
| `fs_count(path)` | Cantidad de entradas, o -1. |

### Archivos

| Función | Descripción |
|---|---|
| `fs_copy(src, dst)` | Copia binaria. Retorna bytes copiados, -1 error, -2 origen inexistente. |
| `fs_touch(path)` | Crea el archivo si no existe (no toca contenido). |
| `fs_remove(path)` | Elimina un archivo. |
| `fs_rename(old, new)` | Renombra o mueve archivo/directorio. |

### Entorno

| Función | Descripción |
|---|---|
| `fs_cwd(buf, len)` | Directorio de trabajo actual. 0 ok, -2 buffer chico. |
| `fs_chdir(path)` | Cambia el directorio de trabajo. |

### Rutas

| Función | Descripción |
|---|---|
| `fs_join(a, b, out, len)` | Une dos rutas con `/` sin dobles separadores. |
| `fs_basename(path, out, len)` | `"a/b.txt"` → `"b.txt"`. |
| `fs_dirname(path, out, len)` | `"a/b.txt"` → `"a"` (o `"."` sin separador). |
| `fs_ext(path, out, len)` | `"datos.tar.gz"` → `"gz"` (sin punto). 1 si había, 0 si no. |

---

## Limitaciones conocidas

- **Frames de stack en Itsuki 5.5.0:** el codegen de Itsuki allocatea `char[N]` como `4·N` bytes de stack y no emite *stack probing*, así que una función con más de ~1000 caracteres totales de arrays locales genera un frame mayor a 4 KB que puede segfaultear en Windows (saltea la *guard page*). Reutilizá buffers y mantenélos chicos hasta que se corrija [upstream](https://github.com/Apolo1061/itsuki-lang/issues). El test de fsuki está escrito respetando este límite.

---

## Mantenedor

Esta librería está hecha y mantenida por [Mateo Collar](https://github.com/mateocollar).

---

## Changelog

Ver [CHANGELOG.md](CHANGELOG.md).

---

## Contribuir

- 🐛 [Reportar bugs](https://github.com/mateocollar/fsuki/issues)
- 🛠️ [Enviar Pull Requests](https://github.com/mateocollar/fsuki/pulls)
- 💬 [Discord de Itsuki](https://discord.gg/K6SEpB7Pf5)

---

## Licencia

MIT. Ver [LICENSE](LICENSE).
