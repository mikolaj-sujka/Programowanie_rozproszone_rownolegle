/**
 * Interfejs dwu-wymiarowej tablicy liczb caĹkowitych. Poprawe pozycje w tablicy
 * to: <br>
 * <ul>
 * <li>Dla wierszy od 0 do rows() - 1
 * <li>Dla kolumn od 0 do cols() - 1
 * </ul>
 */
public interface Table2D {
	/**
	 * Metoda zwraca liczbÄ kolumn w tablicy.
	 * 
	 * @return liczba kolumn
	 */
	int cols();

	/**
	 * Metoda zwraca liczbÄ wierszy w tablicy.
	 * 
	 * @return liczba wierszy
	 */
	int rows();

	/**
	 * WartoĹÄ liczbowa zapisana w tablicy na pozycji position.
	 * 
	 * @param position poĹoĹźenie do odczytu
	 * @return wartoĹÄ liczbowa zapisana w tablicy
	 */
	int get(Position2D position);

	/**
	 * Wpisuje do tablicy 0 na pozycji position.
	 * 
	 * @param position poĹoĹźenie, w ktĂłrym naleĹźy zapisaÄ w tablicy 0
	 */
	void set0(Position2D position);
}