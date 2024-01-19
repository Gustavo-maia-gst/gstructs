from setuptools import setup, Extension

module_name = "gstructs"
module_version = "0.1.0"
author_name = "Gustavo Nogueira"

extension = Extension(
	module_name,
	sources=["src/main.c"]
)

setup(
	name=module_name,
	version=module_version,
	author=author_name,
	ext_modules=[extension]
)
