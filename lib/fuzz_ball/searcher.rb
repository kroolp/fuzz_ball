module FuzzBall
  class Searcher

    attr_reader :files, :files_array, :options, :duple_index

    def initialize(files, opts = {})
      @options     = opts
      @files       = files
      @files_array = files.collect {|f| str2arr(f)}

			@duple_index = DupleIndex.new
			index_duples!
    end

    def search(needle, opts = {})

      needle_ary = str2arr(needle)
      results  = []

      return results if (needle.length < 2)

      decimate_strings!( needle_ary ).each do |candidate|
        smith_waterman(needle_ary, candidate)

        results << {:alignment => @curr_alignment, :score => @curr_score, :string => candidate.pack("U*")}
      end

      if (opts[:order] == :descending)
        results.sort! {|a,b| b[:score] <=> a[:score]}
      else
        results.sort! {|a,b| a[:score] <=> b[:score]}
      end

      results = results.first(opts[:limit]) if opts[:limit].is_a?(Fixnum)

      results
    end

    private

		def index_duples!
			files_array.each_with_index do |str, index|
				duple_index.add(index, str)
			end
		end

    def decimate_strings!(needle)
      max_duples = -1
      grouped_results = {}

      files_array.each do |haystack|
        n_duples   = count_duples(needle, haystack)
        max_duples = n_duples if (n_duples > max_duples)

        grouped_results[n_duples] ||= []
        grouped_results[n_duples] << haystack
      end

      grouped_results[max_duples]
    end

    def str2arr( str )
      if options[:ignore]

        regexp = options[:ignore].collect { |s|
          Regexp.escape(s)
        }.join("|")

        regexp = Regexp.new("(#{regexp})")
        str.gsub(regexp, "").unpack("U*")
      else
        str.unpack("U*")
      end
    end

    def arr2str( arr )
      arr.pack("U*")
    end
  end
end
