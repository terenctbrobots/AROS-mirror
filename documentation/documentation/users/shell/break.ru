=====
Break
=====

.. This document is automatically generated. Don't edit it!

`Index <index>`_ `Prev <beep>`_ `Next <cd>`_ 

---------------

::

 Break 

��� �������
~~~~~~~~~~~
::

     Break <process> [ALL|C|D|E|F]


������
~~~~~~
::

     PROCESS/N,PORT,ALL/S,C/S,D/S,E/S,F/S


������������
~~~~~~~~~~~~
::

     Workbench:c


�������
~~~~~~~
::
     BREAK �������� ���� ��� ��������� �������� �������� CLI. �������� |PROCESS|
     ���������� ����� (ID) �������� CLI, �������� ���������� ��������� ������. 
     ������� STATUS ������� ������������ ���� ���������� CLI-��������� ������ �
     �� ID.
     
     ����� ����� ������� ����� ��� ����� � �������� ������� ������ �����
     ���� ����.
     (You can also specify a public port name and send signal's to the
     port's task.)
     
     ����� �������� ��� ��������� ������� �� ���� ��� ��� ������������� �����
     ALL, � ����� ����� ���������� ������ CTRL-C, CTRL-D, CTRL-E � CTRL-F � 
     ������� ��������������� �����. ����� ����� ������ ID �������� CLI, ����� 
     ������ ������ CTRL-C.


     ��������� ���������� ������� BREAK ��� ��, ��� ��� ������ ����������������
     �������� � ���� ������� � ������� ��������������� ���������� ������.
     

     ������� ���������� ������ ������:
         CTRL-C      -       ���������� �������
         CTRL-D      -       ���������� ������ shell
         CTRL-E      -       ������� ���� ��������
         CTRL-F      -       ������� ���� �������� ��������

     �� ��� ��������� ����������� ���� ��������, ������ ����������� ������ 
     ������������ CTRL-C.
     

�������
~~~~~~~
::

     
     1.SYS:> BREAK 1

         ������� ������ CTRL-C �������� � ������� 1.

     1.SYS:> BREAK 4 E

         ������� ������ CTRL-E �������� � ������� 4.

         
��. �����
~~~~~~~~~
::

     Status


