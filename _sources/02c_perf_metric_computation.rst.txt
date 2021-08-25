Performance Metric Computation
------------------------------
The final stage involves simply computing the desired performance metric. Metric computation is done by a call to ``metric()`` where, the last argument is one of: 

.. hlist::
  :columns: 5

  * ``"FSNR"``
  * ``"NSD"``
  * ``"SFDR"``
  * ``"SINAD"``
  * ``"SNR"``
  * ``"TD"``
  * ``"THD"``
  * ``"TIL``

Again, the test configuration contained in the opaque struct obtained from the first step is helpful in determinging whether the 