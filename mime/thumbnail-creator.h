/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once
#include <QObject>

#include <KIO/ThumbnailCreator>

class PalapeliThumbCreator : public KIO::ThumbnailCreator
{
    Q_OBJECT
public:
    explicit PalapeliThumbCreator(QObject *parent, const QVariantList &args);
    ~PalapeliThumbCreator() override;

    KIO::ThumbnailResult create(const KIO::ThumbnailRequest &request) override;
};
