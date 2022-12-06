from setuptools import Extension, setup

ixprocfs = Extension(
    name='ixprocfs',
    sources=[
        'src/ixprocfs_module/ixprocfs.c',
        'src/ixprocfs_module/diskstats.c',
        'src/ixprocfs_module/diskstats_entry.c',
        'src/ixprocfs_module/proc_fd.c',
        'src/ixprocfs_module/proc_fd_iter.c',
        'src/ixprocfs_module/proc_pid.c',
        'src/ixprocfs_module/proc_pid_entry.c',
        'src/ixprocfs_module/proc_pid_parsers.c',
        'src/ixprocfs_module/proc_pid_iter.c',
	'src/utils/iter.c',
	'src/utils/parser_strings.c'
    ],
    libraries=[
        'bsd',
    ],
)

setup(
    name='pyadmin_tools',
    version='0.0.1',
    ext_modules=[ixprocfs]
)
