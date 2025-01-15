/**
 * Interfejs fabryki wÄtkĂłw
 */
public interface ThreadsFactory {
	/**
	 * Liczba wÄtkĂłw z uprawnieniami od odczytu danych.
	 * 
	 * @return liczba wÄtkĂłw
	 */
	int readersThreads();

	/**
	 * Metoda zwraca obiekt-wÄtek w stanie NEW (nieuruchomiony). WÄtki zwracane sÄ w
	 * parze z poczÄtkowÄ pozycjÄ tablicy, od ktĂłrej powinny rozpoczÄÄ odczyt
	 * danych. Po odebraniu readersThreads wÄtkĂłw kolejne wywoĹanie zwrĂłci null.
	 * 
	 * @param run kod do wykonania w wÄtku
	 * @return wÄtek z prawami do odczytu i jego pozycja startowa
	 */
	ThreadAndPosition readerThread(Runnable run);

	/**
	 * Metoda zwraca obiekt-wÄtek w stanie NEW. WÄtek zwrĂłcony przez tÄ metodÄ jako
	 * jedyny ma prawo zmieniaÄ stan tablicy z danymi. Wszystkie wywoĹania metody
	 * bÄdÄ zwracaÄ ten sam jeden wÄtek.
	 * 
	 * @param run kod do wykonania w wÄtku
	 * @return wÄtek z prawami do zapisu
	 */
	Thread writterThread(Runnable run);
}