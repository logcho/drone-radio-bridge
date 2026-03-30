from setuptools import find_packages, setup

package_name = 'Radio_Node'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='bevyn',
    maintainer_email='evanhennes@gmail.com',
    description='Sender node for two-way communication via usb radio',
    license='Apache License 2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [ 'RadioNode = Radio_Node.Radio_Node:main'
        ],
    },
)
