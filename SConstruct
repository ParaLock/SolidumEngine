import os

def variantglob(env, pattern, ondisk=True, source=True, strings=False,
                recursive=False):
    matches = []

    matches.extend(Dir('#').glob('main.cpp'))

    for root, dirs, filenames in os.walk(env['SOURCE_DIR']):
        cwd = Dir(os.path.join(env['VARIANT_DIR'],
                               os.path.relpath(root, env['SOURCE_DIR'])))
        matches.extend(cwd.glob(pattern, ondisk, source, strings))
    return matches

libraries = ['pthread']
library_paths = ''

# Create Build Environment
env = Environment()
env.Append( CCFLAGS=["-std=gnu++17"] )
env.Append( CCFLAGS=["-pthread"] )

# Customize Environment
env.Replace(VARIANT_DIR='build',
            SOURCE_DIR='Solidum')
#env.Append(CPPPATH=['include'])

# Setup Variant Directory
VariantDir(variant_dir=env['VARIANT_DIR'],
           src_dir=env['SOURCE_DIR'])


# Build the executable
exe = env.Program(os.path.join(env['VARIANT_DIR'], 'SolidumEngine'),
                  variantglob(env, '*.cpp', recursive=True), LIBS = libraries, LIBPATH = library_paths)
