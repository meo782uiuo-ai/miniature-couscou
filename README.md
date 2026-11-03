# Buffer Frame

Mod de [Geode](https://geode-sdk.org) para Geometry Dash (2.2081) que corrige tus clicks tempranos.

Si das un click unos ticks antes de un pincho y vas a morir, el mod prueba ese mismo click un poco más tarde y se queda con el primer retraso con el que sobrevives (hasta 16 ticks por defecto, configurable).

> **Es un cheat de prueba.** No lo uses para rankeds, listas ni verificaciones. Mientras está activo, los niveles se juegan en modo prueba y no se guarda progreso.

## Cómo funciona

1. Al dar un click (salto), se guarda una foto del juego con un checkpoint interno.
2. Si mueres dentro de la ventana de revisión, se cancela la muerte y se vuelve a esa foto.
3. Se repite el intento con el click retrasado 1, 2, 3... ticks.
4. Se queda con el primer retraso con el que sobrevives. Si llega al máximo sin salvarte, mueres normal.

La lógica está en `src/BufferFrame.hpp` (estado) y `src/main.cpp` (hooks).

## Ajustes

| Ajuste | Por defecto | Qué hace |
| --- | --- | --- |
| Activado | sí | Activa o desactiva el mod |
| Retraso máximo (ticks) | 16 | Hasta cuánto puede retrasar un click |
| Ventana de revisión (ticks) | 30 | Cuánto tiempo después del click vigila si mueres |
| Mostrar aviso en pantalla | sí | Etiqueta pequeña mientras está activo |

## Antes de subirlo a GitHub

1. En `mod.json`, cambia `id` (`tuusuario.buffer-frame`) y `developer` por los tuyos.
2. Sube el repositorio a GitHub. El workflow de `.github/workflows/build.yml` compila para Windows, macOS, iOS y Android y deja el `.geode` en los artifacts.

## Compilar en local

Necesitas el [Geode SDK y la CLI](https://docs.geode-sdk.org/getting-started/):

```
geode build
```

## Estado

Versión inicial (v0.1.0). Funciona con la lógica de reintentos por checkpoint, así que:

- Puedes ver pequeños saltos hacia atrás mientras prueba cada retraso.
- Puede fallar en niveles con muchos triggers u objetos en movimiento.
- Los clicks que des durante un reintento no se vuelven a reproducir si hay otro reintento.
- 
