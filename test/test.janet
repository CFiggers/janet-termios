(import janet-termios :as io)

(defn main [&]
  (pp "Get a key prior to any raw term stuff (press key, then Enter)")
  (def key1 (io/await-char))
  (pp "Press the any key to continue...")
  (def key2 (io/stream-char))
  (pp "Press any OTHER key to continue...")
  (var key3 key2)
  (while (= key2 key3) (set key3 (io/stream-char)))
  (pp "done")
  (pp "Get a key after raw term stuff (press key, then enter)")
  (def key4 (io/await-char))
  (pp (string/format "You pushed the '%c' key at first, then '%c', then '%c', and finally '%c'" key1 key2 key3 key4)))