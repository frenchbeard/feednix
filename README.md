Feednix
=======

Feedly client for Linux

## Install

### Distribution Packages
* `Arch Linux <https://aur.archlinux.org/packages/feednix/>`

### From Source

First run autogen.sh script.

Then run your standard make commands. Here is a one liner:

::
./configure && make && sudo make install
::

## Clarification on Sign In Method (PLEASE READ)

Due to the fact that this is open source, the administrators at Feedly have
asked me to use their developer tokens instead of giving me a client secret.
To explain, if I were to use a client secret I would need to distribute it
along with the source code. This is an obvious security risk and I am going
to keep this project open source, hence this new method.

** Unfortunatley, as of now, the Developer tokens have an expiration of 
** three months. This means that you must create a new one every three months.

** I have modified Feednix to help you with this. IT CANNOT DO THIS
** AUTOMATICALLY since developer tokens involve sending you an email
** to retive them.


## Contributing

Please visit this 'page <https://feednix-jarkore.rhcloud.com>' for details.
