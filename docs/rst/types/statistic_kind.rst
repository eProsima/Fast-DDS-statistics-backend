.. include:: ../exports/alias.include

.. _types_statistic_kind:

StatisticKind
=============

Retrieving data from the *eProsima Fast DDS Statistics Backend* with the |get_data-api|,
it is possible to specify the kind of statistic we want to receive in the output.
The available statistics are:

- |StatisticsKind::MEAN-api|: Numerical mean of values in the set.
- |StatisticsKind::STANDARD_DEVIATION-api|: Standard Deviation of the values in the set.
- |StatisticsKind::MAX-api|: Maximum value in the set.
- |StatisticsKind::MIN-api|: Minimum value in the set.
- |StatisticsKind::MEDIAN-api|: Median value of the set.
- |StatisticsKind::COUNT-api|: Amount of values in the set.
- |StatisticsKind::SUM-api|: Summation of the values in the set.
- |StatisticsKind::NONE-api|: Non accumulative kind.
  It chooses a single data point among those in the set.
