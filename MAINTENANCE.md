## Current state

archive is pretty stable, I think it wraps enough of libarchive for our typical use cases.
It could probably be used a bit more automatically in readr / vroom, right now archive is used only by vroom if archive happens to be installed by the user, but most users are not going to know about it and have it installed.

## Known outstanding issues

https://github.com/r-lib/archive/issues/59 mentioned an issue trying to set a C.UTF-8 locale, it seems at least some systems may not have this locale installed, but do have a en_US.UTF-8 locale.
archive needs to set the UTF-8 locale to properly read filenames encoded in UTF-8.
I don't know if it is worth adding code to try multiple locales, or just have the current warning is enough.

## Future directions

In general I don't think there is a lot that needs to be done. Perhaps we could add nicer support for password protected archives, but I think this is not that common in practice.
