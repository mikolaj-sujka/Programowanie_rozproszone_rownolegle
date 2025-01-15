import java.util.Set;

/**
 * GĹĂłwny interfejs programu poszukujÄcego sum w tablicy.
 */
public interface Explorer {
	/**
	 * Ustawienie fabryki wÄtkĂłw
	 * 
	 * @param factory fabryka wÄtkĂłw
	 */
	void setThreadsFactory(ThreadsFactory factory);

	/**
	 * Ustawienie dostÄpu do tablicy z danymi.
	 * 
	 * @param table tablica z danymi
	 */
	void setTable(Table2D table);

	/**
	 * Start poszukiwania sÄsiednich pozycji zawierajÄcych ĹÄczenie wartoĹÄ sum.
	 * 
	 * @param sum poszukiwana suma
	 */
	void start(int sum);

	/**
	 * Wynik dziaĹania programu. W trakcie pracy programu metoda zwraca pusty zbiĂłr.
	 * 
	 * @return zbiĂłr z parami sÄsiednich poĹoĹźeĹ, ktĂłrych suma daĹa wartoĹÄ sum.
	 */
	Set<Pair> result();
}