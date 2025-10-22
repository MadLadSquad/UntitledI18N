# UntitledI18N
[![CI](https://github.com/MadLadSquad/UntitledI18N/actions/workflows/CI.yaml/badge.svg)](https://github.com/MadLadSquad/UntitledI18N/actions/workflows/CI.yaml)
[![MIT license](https://img.shields.io/badge/License-MIT-blue.svg)](https://lbesson.mit-license.org/)
[![Discord](https://img.shields.io/discord/717037253292982315.svg?label=&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)](https://discord.gg/4wgH8ZE)

A library providing a wide variety of [i18n](https://en.wikipedia.org/wiki/Internationalization_and_localization)
abstractions and interfaces to query system and user language and locale settings.

Features:
- [x] Full C and C++ API for all features and abstractions
- [x] Translation API
    - [x] Static string interpolation
    - [x] Interpolation using positional arguments
    - [x] Dynamic string interpolation
    - [x] Conditional string interpolation


## The custom translation format
One of the distinctive features of the library is the custom translation file format. It's a YAML-based format that 
allows for most features, that translators and developers, need.

While many would choose a system like [GNU gettext](https://www.gnu.org/software/gettext/), we think that it's somewhat
limited. Additionally, GNU's LGPL licensing and the availability of only 1 cross-platform implementation, made us
rethink choosing gettext. Other implementations do exist, namely 
[NetBSD's libintl implementation](https://github.com/NetBSD/src/tree/trunk/lib/libintl), however, we saw that
it would be too time-consuming for us to port this to Linux and Windows.

The benefits of our system are detailed below.

### YAML is standard
There are many benefits for using a standard file format like YAML or JSON. One of them is that, if we decided to write 
future applications in another programming language, we don't have to worry about rewriting the whole text format
parsing library. Additionally, YAML is highly readable and can be learned in about 10 minutes.

### Message IDs are actual IDs
Like gettext, we use string message IDs to get translations. Unlike gettext, these IDs are actual IDs. The IDs should
be written with a descriptive name like "close-button". Once placed in a translation template file, these IDs can be
translated into any language, including "en-US". Using our predefined language codes, an application can further set the
default language in case a fallback is needed.

This solves the following problems:
1. Changing the source string in the file invalidates the whole translation ID, which breaks translations. We prefer 
outdated translations than having none at all
1. Fixes the need to use or provide additional context variables since the IDs can already include that information - 
e.g. a button and status message may have the same ID in gettext, "Open". In other languages this will require 
completely different words, which in gettext, is solved by adding context. In our system one may define 
"open-button-action" and "open-status-message", which can easily be translated separately.

This may not seem like it makes much of a difference, but having these generic IDs allows us to follow the separation of
concerns principle and does not bother the developer with translation concerns and vice-versa.

### Terms
The format also supports the ability for the user to define so-called "terms". These "terms" can be used for addresses,
place names and general branding without needing to be translated.

### Variables, templating and interpolation
The format also supports the ability to use variables to template strings. Since every ID defines a variable, it's easy
to chain translations when needed. Variables can also be pushed from code for purposes of string interpolation.

The format also allows for conditional templating, so that the translator can easily deal with genders, cases, plurals
and other grammatical concepts.

## Installation and Learning
All documentation, including install instructions, can be found on the 
[wiki](https://github.com/MadLadSquad/UntitledI18N/wiki).
