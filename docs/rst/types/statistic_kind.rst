.. include:: ../exports/alias.include

.. _types_statistic_kind:

StatisticKind
=============

|get_data-api| retrieves data from the *eProsima Fast DDS Statistics Backend*
for the requested kind of statistic.
The available statistics are:

- |StatisticsKind::MEAN-api|: Numerical mean of values in the set.
- |StatisticsKind::STANDARD_DEVIATION-api|: Standard deviation of the values in the set.
- |StatisticsKind::MAX-api|: Maximum value in the set.
- |StatisticsKind::MIN-api|: Minimum value in the set.
- |StatisticsKind::MEDIAN-api|: Median value of the set.
- |StatisticsKind::COUNT-api|: Number of values in the set.
- |StatisticsKind::SUM-api|: Summation of the values in the set.
- |StatisticsKind::NONE-api|: Non accumulative kind.
  It takes a single data point from the set: the first one.
